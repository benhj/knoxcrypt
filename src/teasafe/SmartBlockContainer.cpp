#include "teasafe/SmartBlockContainer.hpp"

namespace teasafe
{

    SmartBlockContainer::SmartBlockContainer(SharedCoreIO const &io,
                                             uint64_t const rootBlockIndex,
                                             OpenDisposition const &openDisposition)
        : m_io(io)
        , m_rootBlockIndex(rootBlockIndex)
        , m_openDisposition(openDisposition)
        , m_size(countBlocks())
        , m_workingBlock()
    {

    }

    void
    SmartBlockContainer::push_back(FileBlock const &)
    {
        ++m_size;
    }

    FileBlock
    SmartBlockContainer::operator[](size_t const n)
    {
        FileBlock block = findBlock(n);
        if(!m_workingBlock) {
            m_workingBlock = block;
        }
        return findBlock(n);
    }

    size_t
    SmartBlockContainer::size()
    {
        return m_size;
    }

    FileBlock
    SmartBlockContainer::findBlock(size_t const n)
    {
        FileBlock block(m_io,
                        m_rootBlockIndex,
                        m_openDisposition);

        if(n == 0) {
            return block;
        }

        uint64_t nextBlock = block.getNextIndex();

        size_t c(0);

        for (;;) {
            FileBlock newBlock(m_io,
                               nextBlock,
                               m_openDisposition);

            nextBlock = newBlock.getNextIndex();
            if(c == n) {
                return newBlock;
            }
            ++c;
        }
    }

    size_t
    SmartBlockContainer::countBlocks()
    {
        // find very first block
       FileBlock block(m_io,
                       m_rootBlockIndex,
                       m_openDisposition);

       uint64_t cBlock = m_rootBlockIndex;

       uint64_t nextBlock = block.getNextIndex();
       ++m_size;

       // seek to the very end block
       while (nextBlock != cBlock) {

           cBlock = nextBlock;
           FileBlock newBlock(m_io,
                              cBlock,
                              m_openDisposition);

           nextBlock = newBlock.getNextIndex();
           ++m_size;
       }
    }


}
