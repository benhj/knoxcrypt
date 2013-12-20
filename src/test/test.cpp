#include "test/BFSTest.hpp"
#include "test/FileBlockTest.hpp"
#include "test/FileEntryTest.hpp"
#include "test/FileEntryDeviceTest.hpp"
#include "test/MakeBFSTest.hpp"
#include "test/FolderEntryTest.hpp"
#include "test/TestHelpers.hpp"


int main()
{
    BFSTest();
    MakeBFSTest();
    FileEntryDeviceTest();
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
