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

#include "knoxcrypt/CoreKnoxCryptIO.hpp"
#include "utility/CipherCallback.hpp"
#include "utility/EcholessPasswordPrompt.hpp"
#include "utility/EventType.hpp"
#include "utility/MakeKnoxCrypt.hpp"
#include "cryptostreampp/Algorithms.hpp"
#include "cryptostreampp/RandomNumberGenerator.hpp"

#include <boost/progress.hpp>
#include <boost/program_options.hpp>

#include <ctime>
#include <functional>
#include <iostream>
#include <string>

void imagerCallback(knoxcrypt::EventType eventType, long const amount)
{
    static std::shared_ptr<boost::progress_display> pd;
    if(eventType == knoxcrypt::EventType::ImageBuildStart) {
        std::cout<<"Building main fs image.."<<std::endl;
        pd = std::make_shared<boost::progress_display>(amount);
    }
    if(eventType == knoxcrypt::EventType::ImageBuildUpdate) {
        ++(*pd);
    }
    if(eventType == knoxcrypt::EventType::IVWriteEvent) {
        std::cout<<"Writing out IV.."<<std::endl;
    }
    if(eventType == knoxcrypt::EventType::RoundsWriteEvent) {
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
        ("imageName", po::value<std::string>(), "knoxcrypt image path")
        ("blockCount", po::value<uint64_t>(), "size of filesystem in 4096 blocks (12800 = 50MB)")
        ("coffee", po::value<bool>(&magicPartition)->default_value(false), "create alternative sub-volume")
        ("sparse", po::value<bool>(&sparse)->default_value(false), "create a sparse image")
        ("cipher", po::value<std::string>(&cipher)->default_value("aes"), "the cipher type used");

    po::positional_options_description positionalOptions;
    (void)positionalOptions.add("imageName", 1);
    (void)positionalOptions.add("blockCount", 1);

    auto io(std::make_shared<knoxcrypt::CoreKnoxCryptIO>());
    io->useBlockCache = false;

    // use a non-deterministic random device to generate the iv. This
    // allegedly pulls data from /dev/urandom
    io->encProps.iv = cryptostreampp::crypto_random();
    io->encProps.iv2 = cryptostreampp::crypto_random();
    io->encProps.iv3 = cryptostreampp::crypto_random();
    io->encProps.iv4 = cryptostreampp::crypto_random();

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
            std::cout<<"initialization vector A: "<<io->encProps.iv<<std::endl;
            std::cout<<"initialization vector B: "<<io->encProps.iv2<<std::endl;
            std::cout<<"initialization vector C: "<<io->encProps.iv3<<std::endl;
            std::cout<<"initialization vector D: "<<io->encProps.iv4<<std::endl;
            std::cout<<"Encryption algorithm: "<<cipher<<std::endl;
        }
    } catch (...) {
        std::cout<<"Problem parsing options"<<std::endl;
        std::cout<<desc<<std::endl;
        return 1;
    }

    auto blocks = vm["blockCount"].as<uint64_t>();

    io->path = vm["imageName"].as<std::string>().c_str();
    io->blocks = blocks;
    io->freeBlocks = blocks;
    io->encProps.password.append(knoxcrypt::utility::getPassword("knoxcrypt password: "));
    io->rounds = 64; // obsolete (not currently used; used to be used by XTEA)

    if(cipher == "aes") {
        io->encProps.cipher = cryptostreampp::Algorithm::AES;
    } else if(cipher == "twofish") {
        io->encProps.cipher = cryptostreampp::Algorithm::Twofish;
    } else if(cipher == "serpent") {
        io->encProps.cipher = cryptostreampp::Algorithm::Serpent;
    } else if(cipher == "rc6") {
        io->encProps.cipher = cryptostreampp::Algorithm::RC6;
    } else if(cipher == "mars") {
        io->encProps.cipher = cryptostreampp::Algorithm::MARS;
    } else if(cipher == "cast256") {
        io->encProps.cipher = cryptostreampp::Algorithm::CAST256;
    } else if(cipher == "camellia") {
        io->encProps.cipher = cryptostreampp::Algorithm::Camellia;
    } else if(cipher == "rc5") {
        io->encProps.cipher = cryptostreampp::Algorithm::RC5;
    } else if(cipher == "shacal2") {
        io->encProps.cipher = cryptostreampp::Algorithm::SHACAL2;
    } else if(cipher == "null") {
        io->encProps.cipher = cryptostreampp::Algorithm::NONE;
    }

    // magic partition?
    knoxcrypt::OptionalMagicPart omp;
    if (magicPartition) {

        auto partBlock = atoi(knoxcrypt::utility::getPassword("sub-volume root block: ").c_str());
        if(partBlock == 0 || partBlock >= blocks) {
            std::cout<<"Error: sub-volume root block must be less than "<<blocks<<" AND greater than 0"<<std::endl;
            return 1;
        }

        omp = knoxcrypt::OptionalMagicPart(partBlock);
    }

    // register progress call back for cipher
    long const amount = knoxcrypt::detail::CIPHER_BUFFER_SIZE / 100000;
    auto f(std::bind(&knoxcrypt::cipherCallback, std::placeholders::_1, amount));
    io->ccb = f;

    knoxcrypt::MakeKnoxCrypt imager(io, sparse, omp);

    // register progress callback for imager
    auto fb(std::bind(&imagerCallback, std::placeholders::_1, io->blocks));
    imager.registerSignalHandler(fb);
    imager.buildImage();

    return 0;
}
