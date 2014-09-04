/*
  Copyright (c) <2013-2014>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "teasafe/CoreTeaSafeIO.hpp"
#include "utility/CipherCallback.hpp"
#include "utility/EcholessPasswordPrompt.hpp"
#include "utility/EventType.hpp"
#include "utility/MakeTeaSafe.hpp"
#include "utility/RandomNumberGenerator.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/progress.hpp>
#include <boost/program_options.hpp>
#include <boost/random.hpp>
#include <boost/nondet_random.hpp>

#include <ctime>
#include <iostream>
#include <string>

void imagerCallback(teasafe::EventType eventType, long const amount)
{
    static boost::shared_ptr<boost::progress_display> pd;
    if(eventType == teasafe::EventType::ImageBuildStart) {
        std::cout<<"Building main fs image.."<<std::endl;
        pd = boost::make_shared<boost::progress_display>(amount);
    }
    if(eventType == teasafe::EventType::ImageBuildUpdate) {
        ++(*pd);
    }
    if(eventType == teasafe::EventType::IVWriteEvent) {
        std::cout<<"Writing out IV.."<<std::endl;
    }
    if(eventType == teasafe::EventType::RoundsWriteEvent) {
        std::cout<<"Writing out number of encryption rounds.."<<std::endl;
    }
}

int main(int argc, char *argv[])
{

    namespace po = boost::program_options;
    bool magicPartition;
    bool sparse;
    std::string cipher;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "teasafe image path")
        ("blockCount", po::value<uint64_t>(), "size of filesystem in 4096 blocks (12800 = 50MB)")
        ("coffee", po::value<bool>(&magicPartition)->default_value(false), "create alternative sub-volume")
        ("sparse", po::value<bool>(&sparse)->default_value(false), "create a sparse image")
        ("cipher", po::value<std::string>(&cipher)->default_value("aes"), "the cipher type used");

    po::positional_options_description positionalOptions;
    (void)positionalOptions.add("imageName", 1);
    (void)positionalOptions.add("blockCount", 1);

    teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());

    // use a non-deterministic random device to generate the iv. This
    // allegedly pulls data from /dev/urandom
    io->iv = teasafe::utility::random();
    io->iv2 = teasafe::utility::random();
    io->iv3 = teasafe::utility::random();
    io->iv4 = teasafe::utility::random();

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(positionalOptions).run(),
                  vm);
        po::notify(vm);
        if (vm.count("help") ||
            vm.count("imageName")==0 || vm.count("blockCount") == 0) {
            std::cout << desc << std::endl;
            return 1;
        }

        if (vm.count("help")) {
            std::cout<<desc<<"\n";
        } else {

            if(sparse && magicPartition) {
                std::cout<<"Error: sparse volumes with coffee mode not supported"<<std::endl;
                exit(0);
            }

            std::cout<<"image path: "<<vm["imageName"].as<std::string>()<<std::endl;
            std::cout<<"file system size in blocks: "<<vm["blockCount"].as<uint64_t>()<<std::endl;
            std::cout<<"initialization vector A: "<<io->iv<<std::endl;
            std::cout<<"initialization vector B: "<<io->iv2<<std::endl;
            std::cout<<"initialization vector C: "<<io->iv3<<std::endl;
            std::cout<<"initialization vector D: "<<io->iv4<<std::endl;
            std::cout<<"Encryption algorithm: "<<cipher<<std::endl;
        }
    } catch (...) {
        std::cout<<"Problem parsing options"<<std::endl;
        std::cout<<desc<<std::endl;
        return 1;
    }

    uint64_t blocks = vm["blockCount"].as<uint64_t>();

    io->path = vm["imageName"].as<std::string>().c_str();
    io->blocks = blocks;
    io->freeBlocks = blocks;
    io->password.append(teasafe::utility::getPassword("teasafe password: "));
    io->rounds = 64; // obsolete (not currently used; used to be used by XTEA)
    io->cipher = 1; // AES

    if(cipher == "aes") {
        io->cipher = 1; 
    } else if(cipher == "twofish") {
        io->cipher = 2;
    } else if(cipher == "serpent") {
        io->cipher = 3;
    } else if(cipher == "rc6") {
        io->cipher = 4;
    } else if(cipher == "mars") {
        io->cipher = 5;
    } else if(cipher == "cast256") {
        io->cipher = 6;
    } else if(cipher == "camellia") {
        io->cipher = 7;
    }

    // magic partition?
    teasafe::OptionalMagicPart omp;
    if (magicPartition) {

        unsigned long partBlock = atoi(teasafe::utility::getPassword("sub-volume root block: ").c_str());
        if(partBlock == 0 || partBlock >= blocks) {
            std::cout<<"Error: sub-volume root block must be less than "<<blocks<<" AND greater than 0"<<std::endl;
            return 1;
        }

        omp = teasafe::OptionalMagicPart(partBlock);
    }

    // register progress call back for cipher
    long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
    boost::function<void(teasafe::EventType)> f(boost::bind(&teasafe::cipherCallback, _1, amount));
    io->ccb = f;

    teasafe::MakeTeaSafe imager(io, sparse, omp);

    // register progress callback for imager
    boost::function<void(teasafe::EventType)> fb(boost::bind(&imagerCallback, _1, io->blocks));
    imager.registerSignalHandler(fb);
    imager.buildImage();

    return 0;
}
