
// This file is part of lipstick, a QML desktop library
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation
// and appearing in the file LICENSE.LGPL included in the packaging
// of this file.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// Copyright (c) 2014, Sami Kananoja <sami.kananoja@jolla.com>

#include <QFile>

#include "launcherwatchermodel.h"
#include "launcheritem.h"

LauncherWatcherModel::LauncherWatcherModel(QObject *parent) :
    QObjectListModel(parent),
    _fileSystemWatcher()
{
    connect(&_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(monitoredFileChanged(QString)));
}

LauncherWatcherModel::~LauncherWatcherModel()
{
}

void LauncherWatcherModel::monitoredFileChanged(const QString &changedPath)
{
    if (!QFile(changedPath).exists()) {
        foreach (LauncherItem *item, *getList<LauncherItem>()) {
            if (item->filePath() == changedPath) {
                removeItem(item);
                emit filePathsChanged();
                break;
            }
        }
    }
}

QStringList LauncherWatcherModel::filePaths()
{
    QStringList paths;
    foreach (LauncherItem *item, *getList<LauncherItem>()) {
        paths.append(item->filePath());
    }
    return paths;
}

void LauncherWatcherModel::setFilePaths(QStringList paths)
{
    QString oldPaths = filePaths().join(',');
    reset();
    QStringList addedPaths;
    foreach (QString path, paths) {
        if (!addedPaths.contains(path)) {
            addItemIfValid(path);
            // we don't really care whether it was added or not,
            // this is just for avoiding duplicates to be added
            addedPaths.append(path);
        }
    }
    QString newPaths = filePaths().join(',');
    if (newPaths != oldPaths) {
        emit filePathsChanged();
    }
}

void LauncherWatcherModel::addItemIfValid(const QString &path)
{
    LAUNCHER_DEBUG("Creating LauncherItem for desktop entry" << path);
    LauncherItem *item = new LauncherItem(path, this);

    bool isValid = item->isValid();
    if (isValid) {
        addItem(item);
        _fileSystemWatcher.addPath(path);
    } else {
        LAUNCHER_DEBUG("Item" << path << "is not valid");
        delete item;
        item = NULL;
    }
}
