#ifndef TREEITEMPATHDERIVER_H
#define TREEITEMPATHDERIVER_H

#include <QTreeWidgetItem>

#include <vector>

namespace detail {

    inline
    std::string getPathFromCurrentItem_(QTreeWidgetItem *item)
    {
        std::string str("");
        if(item->parent()) {
            str.append(getPathFromCurrentItem_(item->parent()));
            str.append(item->text(0).toStdString());
            str.append("/");
            return str;
        }
        return str.append(item->text(0).toStdString());
    }

    inline
    std::string getPathFromCurrentItem(QTreeWidgetItem *item)
    {
        std::string fullPath = getPathFromCurrentItem_(item);
        if(!fullPath.empty()) {
            return std::string(fullPath.begin(), fullPath.end() - 1);
        }

        return fullPath;
    }

}

#endif // TREEITEMPATHDERIVER_H
