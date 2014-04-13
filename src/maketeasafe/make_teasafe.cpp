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
#include "utility/EcholessPasswordPrompt.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <ctime>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{

    namespace po = boost::program_options;
    bool magicPartition;
    unsigned int rounds;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "teasafe image path")
        ("blockCount", po::value<uint64_t>(), "size of filesystem in blocks")
        ("coffee", po::value<bool>(&magicPartition)->default_value(false), "create alternative sub-volume")
        ("rounds", po::value<unsigned int>(&rounds)->default_value(64), "number of encryption rounds");

    po::positional_options_description positionalOptions;
    (void)positionalOptions.add("imageName", 1);
    (void)positionalOptions.add("blockCount", 1);

    teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());
    io->iv = time(NULL);
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

            std::cout<<"image path: "<<vm["imageName"].as<std::string>()<<std::endl;
            std::cout<<"file system size in blocks: "<<vm["blockCount"].as<uint64_t>()<<std::endl;
            std::cout<<"number of encryption rounds: "<<vm["rounds"].as<unsigned int>()<<std::endl;
            std::cout<<"initialization vector: "<<io->iv<<std::endl;
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

    io->rounds = rounds;

    // magic partition?
    teasafe::OptionalMagicPart omp;
    if (magicPartition) {
        omp = teasafe::OptionalMagicPart(atoi(teasafe::utility::getPassword("sub-volume root block: ").c_str()));
    }

    teasafe::MakeTeaSafe teasafe(io, omp);

    return 0;
}
