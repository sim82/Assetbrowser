#include <QVector>
#include <QDir>
#include <QDirIterator>
#include <QStack>
#include <iostream>

#include "assetcollectionoutlinemodel.h"
#include <assetcollection.h>
class CollectionDir {
public:
    CollectionDir(const QString &name, CollectionDir *parent, AssetCollection *collection)
        : name_(name)
        , parent_(parent)
        , collection_(collection)
        , nonemptySubdirs(-1)
        , emptyState(-1)
    {}

    void addSubDir(CollectionDir *dir)
    {
        subDirs_.append(dir);
    }


    QString initPrefix(CollectionDir *stop = nullptr)
    {
        CollectionDir *d = this;

        while( d != stop )
        {
            if(!prefix_.isEmpty())
            {
                prefix_.prepend( "/" );
            }
            prefix_.prepend(d->name_);
            d = d->parent_;
        }

        return prefix_;
    }

    int numNonemptySubdirs()
    {
        if( nonemptySubdirs == -1 )
        {
            nonemptySubdirs = 0;
            for( auto it = subDirs_.begin(), eit = subDirs_.end(); it != eit; ++it )
            {
                if(!(*it)->isEmpty())
                {
                    ++nonemptySubdirs;
                }
            }

        }

        return nonemptySubdirs;

    }

    CollectionDir *nonemptySubdir(int n)
    {
        int i = 0;
        for( auto it = subDirs_.begin(), eit = subDirs_.end(); it != eit; ++it )
        {
            CollectionDir *dir = *it;
            if(dir->isEmpty())
            {
                continue;
            }

            if(i == n)
            {
                return dir;
            }
            ++i;
        }

        throw std::runtime_error( "bad nonemptySubdir lookup");
    }

    bool isEmpty() {
        if( emptyState == -1 )
        {
            emptyState = numNonemptySubdirs() == 0 && collection_->numIdsForPrefix(prefix_) == 0 ? 1 : 0;
        }

        return emptyState == 1;
    }

    CollectionDir *parent()
    {
        return parent_;
    }

    const QString &name()
    {
        return name_;
    }
    const QString &prefix()
    {
        return prefix_;
    }

private:
    QString name_;
    QString prefix_;
    CollectionDir *parent_;
    AssetCollection *collection_;
    QVector<CollectionDir *> subDirs_;

    int nonemptySubdirs;
    int emptyState;
};

AssetCollectionOutlineModel::AssetCollectionOutlineModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

AssetCollectionOutlineModel::~AssetCollectionOutlineModel()
{

}

void AssetCollectionOutlineModel::addCollection(AssetCollection *collection)
{
    collections.append(collection);

    QDir baseDir = collection->baseDir();
    CollectionDir *root = new CollectionDir(baseDir.path(), nullptr, collection);

    QStack <std::pair<QDirIterator *, CollectionDir *>>stack;

    const QDir::Filters filters = QDir::NoDotAndDotDot | QDir::Dirs;

    stack.push(std::make_pair(new QDirIterator(baseDir.absolutePath(), filters), root));


    while( !stack.empty() )
    {
        std::pair<QDirIterator *, CollectionDir *> &top = stack.top();
        QDirIterator *it = top.first;

        if( !it->hasNext() )
        {
            delete it;
            stack.pop();
            continue;
        }

        QString absoluteFilename = it->next();
        QDirIterator *nextit = new QDirIterator(absoluteFilename, filters);

        QString relativeFilename = it->fileName();
        CollectionDir *nextcd = new CollectionDir(relativeFilename, top.second, collection );

        dirs.append(nextcd);
        top.second->addSubDir(nextcd);
        stack.push(std::make_pair(nextit, nextcd));

        std::cout << "dir: " << nextcd->initPrefix(root).toStdString() << std::endl;

    }

    dirs.append(root);
    rootDirs.append(root);
}

QModelIndex AssetCollectionOutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
    {
        return QModelIndex();
    }
    // get the parent node
//    QFileSystemModelPrivate::QFileSystemNode *parentNode = (d->indexValid(parent) ? d->node(parent) :
//                                                   const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&d->root));
//    Q_ASSERT(parentNode);

//    // now get the internal pointer for the index
//    QString childName = parentNode->visibleChildren[d->translateVisibleLocation(parentNode, row)];
//    const QFileSystemModelPrivate::QFileSystemNode *indexNode = parentNode->children.value(childName);
//    Q_ASSERT(indexNode);

//    return createIndex(row, column, const_cast<QFileSystemModelPrivate::QFileSystemNode*>(indexNode));

    if( !parent.isValid())
    {
        return createIndex(row, column, rootDirs.at(row));
    }
    else
    {
        CollectionDir *dir = reinterpret_cast<CollectionDir *> (parent.internalPointer());
        return createIndex(row, column, dir->nonemptySubdir(row));
    }
}

QModelIndex AssetCollectionOutlineModel::parent(const QModelIndex &child) const
{
    if( !child.isValid() )
    {
        return QModelIndex();
    }
    else
    {
        CollectionDir *dir = reinterpret_cast<CollectionDir *> (child.internalPointer());

        if( dir->parent() == nullptr )
        {
            return QModelIndex();
        }
        else
        {
            for( int i = 0; i < dir->parent()->numNonemptySubdirs(); ++i )
            {
                if( dir == dir->parent()->nonemptySubdir(i))
                {
                    return createIndex(i, 0, dir->parent() );
                }
            }
            return QModelIndex();
        }
    }
}

int AssetCollectionOutlineModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return collections.size();
    }
    else {
        CollectionDir *dir = reinterpret_cast<CollectionDir *>(parent.internalPointer());
        //return dir->subDirs_.size();
        return dir->numNonemptySubdirs();
    }
}

int AssetCollectionOutlineModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 1;
}

QVariant AssetCollectionOutlineModel::data(const QModelIndex &index, int role) const
{
    if( role == Qt::DisplayRole )
    {

        CollectionDir *dir = reinterpret_cast<CollectionDir *>(index.internalPointer());

        QString name = dir->name();
        return name;
    }
    else if( role == Qt::UserRole )
    {
        CollectionDir *dir = reinterpret_cast<CollectionDir *>(index.internalPointer());

        return dir->prefix();
    }
    else
    {
        return QVariant();
    }
}

QVector<QString> AssetCollectionOutlineModel::prefixList()
{
    QVector<QString> list;
    for( auto it = dirs.begin(), eit = dirs.end(); it != eit; ++it )
    {
        CollectionDir *dir = (*it);

        if( dir->isEmpty())
        {
            continue;

        }
        list.append(dir->prefix());
    }

    return list;
}

