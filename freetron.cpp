/*
 * Freetron - an open-source software scantron implementation
 *
 * Todo:
 *   - Statistics (e.g. most missed, least missed, ...?) as text and/or images
 *   - Generalize for any style of form, make forms/type*.xml and autodetect
 *   - When a box is missing, calculate supposed position
 *   - Pick largest image on the page of a PDF
 *   - Rotate based on a few boxes in line, then find all boxes
 *   - Auto-adjusting HEIGHT_ERROR and MIN_BLACK
 */

#include <list>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <IL/il.h>

#include "options.h"
#include "processor.h"

#ifndef NOSITE
#include <unistd.h>
#include <cppcms/service.h>
#include <cppcms/mount_point.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>

#include "website/website.h"
#include "website/rpc.h"
#endif

// This must be global since we're using extern in options.h. This is used all
// over the place to enable outputting debug images.
bool DEBUG = false;

enum class Args
{
    Unknown,
    Help,
    Threads,
    Debug,
    Quiet,
    ID,
    Daemon
};

void help()
{
    std::cerr << "Usage:" << std::endl
              << "  freetron [options] --daemon path/to/files" << std::endl
              << "  freetron [options] -i KeyID in.pdf" << std::endl
              << std::endl
              << "  Options" << std::endl
              << "    -i, --id          ID of form to use as the key (if not daemon)" << std::endl
              << "    -q, --quiet       don't print error messages (not implemented)" << std::endl
              << "    -d, --debug       output debug images" << std::endl
              << "    -t, --threads #   max number of threads to create" << std::endl
              << "    --daemon path/    run the website, don't exit till Ctrl+C" << std::endl;
}

void invalid()
{
    std::cerr << "Invalid argument (see \"-h\" for usage)" << std::endl;
    std::exit(1);
}

int main(int argc, char* argv[])
{
    // Argument parsing
    std::string path;
    std::string filename;
    bool daemon = false;
    bool quiet = false;
    int threads = 0; // 0 == number of cores
    long long key = DefaultID;

    std::map<std::string, Args> options = {{
        { "-h",        Args::Help },
        { "--help",    Args::Help },
        { "-i",        Args::ID },
        { "--id",      Args::ID },
        { "-t",        Args::Threads },
        { "--threads", Args::Threads },
        { "-d",        Args::Debug },
        { "--debug",   Args::Debug },
        { "-q",        Args::Quiet },
        { "--quiet",   Args::Quiet },
        { "--daemon",  Args::Daemon }
    }};

    for (int i = 1; i < argc; ++i)
    {
        switch (options[argv[i]])
        {
            case Args::Help:
                help();
                return 1;
            case Args::ID:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    key = std::stoll(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: invalid key ID" << std::endl;
                    return 1;
                }
                catch (const std::out_of_range&)
                {
                    std::cerr << "Error: key ID too long" << std::endl;
                    return 1;
                }
                break;
            case Args::Threads:
                ++i;

                if (i == argc)
                    invalid();

                try
                {
                    threads = std::stoi(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: invalid number of threads" << std::endl;
                    return 1;
                }
                catch (const std::out_of_range&)
                {
                    std::cerr << "Error: max threads too long" << std::endl;
                    return 1;
                }
                break;
            case Args::Debug:
                DEBUG = true;
                break;
            case Args::Quiet:
                quiet = true;
                break;
            case Args::Daemon:
                ++i;
                daemon = true;

                if (i == argc)
                    invalid();

                path = argv[i];
                break;
            default:
                if (!filename.empty())
                    invalid();

                filename = argv[i];
                break;
        }
    }


    if (key == DefaultID && !daemon)
    {
        std::cerr << "Error: key ID cannot be the default ID" << std::endl;
        return 1;
    }

    ilInit();
    Processor p(threads);

    if (!daemon)
    {
        // Process a single form and exit
        p.add(0, key, filename);
        p.wait();
        std::cout << p.print(0);
    }
    else
    {
#ifndef NOSITE
        try
        {
            // Go to the website directory
            if (chdir(path.c_str()) != 0)
            {
                throw std::runtime_error("Couldn't set directory");
            }

            int count = 3;
            char args1[] = "-c";
            char args2[] = "config.js";
            char* args[] = { argv[0], args1, args2 };

            // Init website
            cppcms::service srv(count, args);

            srv.applications_pool().mount(
                cppcms::applications_factory<website>(),
                cppcms::mount_point("/website")
            );

            srv.applications_pool().mount(
                cppcms::applications_factory<rpc>(),
                cppcms::mount_point("/rpc")
            );

            // Look for any leftover forms in the upload directory

            srv.run();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            p.exit();
        }
#else
        p.exit();
        std::cerr << "Error: Website disabled when compiled" << std::endl;
#endif
    }

    return 0;
}
