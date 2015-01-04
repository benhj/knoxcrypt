/*
  Copyright (c) <2014>, <BenHJ>
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

#pragma once

#include <boost/format.hpp>

#include <ctime>
#include <string>
#include <vector>

namespace simpletest {

    int testFailures = 0;
    int passedPoints = 0;
    std::vector<std::string> failingTestPoints;

    /// in the function, we compare on A and B and give a comment string, C
    /// For example
    /// ASSERT_EQUAL(a, b, "Test point");
    /// will compare the values of a and b
    template <typename A, typename B>
    void ASSERT_EQUAL(A const &a, B const &b, std::string const &c)    
    {                                     
        if(a == b) {                                                       
            std::cout<<boost::format("%1% %|100t|%2%\n") % c % "passed";  
            ++passedPoints;                                               
        } else {                                                          
            std::cout<<boost::format("%1% %|100t|%2%\n") % c % "failed";  
            ++testFailures;                                                
            failingTestPoints.push_back(c);                               
        }
    }

    // a corresponding unequal function
    template <typename A, typename B>
    void ASSERT_UNEQUAL(A const &a, B const &b, std::string const &c)    
    {                                     
        if(a != b) {                                                       
            std::cout<<boost::format("%1% %|100t|%2%\n") % c % "passed";  
            ++passedPoints;                                               
        } else {                                                          
            std::cout<<boost::format("%1% %|100t|%2%\n") % c % "failed";  
            ++testFailures;                                                
            failingTestPoints.push_back(c);                               
        }
    }

    void showResults()
    {
        std::cout<<"\n\nThere were "<<testFailures<<"/"<<(passedPoints + testFailures)<<" assertion failures\n\n"<<std::endl;
        if (testFailures > 0) {
            std::cout<<"The failures were:\n\n"<<std::endl;
            for (auto const & it : failingTestPoints) {
                std::cout<<it<<std::endl;
            }
            std::cout<<"\n\n"<<std::endl;
        }
    }
}

