#include <gtest/gtest.h>

#include "setupfilehandler.h"


TEST(setupFileHandlerTest, Constructor)
{
    QDir dir(QDir::currentPath());
    QString testFilePath = dir.filePath("testSetupFile.ini");
    qDebug("testFilePath = %s", qPrintable(testFilePath));
    
    if (QFile::exists(testFilePath)) {
        qDebug("Removing existing test file.");
        QFile::remove(testFilePath);
    }
    
    setupFileHandler handler(testFilePath);
    
    EXPECT_EQ(handler.getSetupFileName(), testFilePath);
}