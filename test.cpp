//#include "BFSEntryWriterTest.hpp"
#include "FileBlockTest.hpp"
#include "FileEntryTest.hpp"
#include "MakeBFSTest.hpp"
#include "FolderEntryTest.hpp"
#include "TestHelpers.hpp"


int main()
{
    MakeBFSTest();
    FileBlockTest();
    FileEntryTest();
    FolderEntryTest();

    std::cout<<"\n\nThere were "<<testFailures<<"/"<<passedPoints<<" assertion failures\n\n"<<std::endl;
    if (testFailures > 0) {
        std::cout<<"The failures were:\n\n"<<std::endl;
        std::vector<std::string>::iterator it = failingTestPoints.begin();
        for (; it != failingTestPoints.end(); ++it) {
            std::cout<<*it<<std::endl;
        }
        std::cout<<"\n\n"<<std::endl;
    }
}
