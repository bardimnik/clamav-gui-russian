#include <QTest>
#include <QSharedPointer>
#include <QDebug>
#include <QDir>

#include "setupfilehandler.h"


class SetupFileHandlerTest : public QObject
{
    Q_OBJECT
private:
    QSharedPointer<setupFileHandler> handler;
    QString _testFilePath;
private slots:
    void initTestCase()
    {
        QDir dir(QDir::currentPath());
        _testFilePath = dir.filePath("testSetupFile.ini");
        qDebug("testFilePath = %s", qPrintable(_testFilePath));
        if (QFile::exists(_testFilePath)) {
            qDebug("Removing existing test file.");
            QFile::remove(_testFilePath);
        }
        handler = QSharedPointer<setupFileHandler>(new setupFileHandler(_testFilePath));
    }

    void testConstructor()
    {
        qDebug("Testing constructor.");
        QVERIFY(handler != nullptr);
        QVERIFY(handler->getSetupFileName() == _testFilePath);
    }

    void cleanupTestCase()
    {
        qDebug("Called after SetupFileHandlerTest.");
    }
};


QTEST_MAIN(SetupFileHandlerTest)
#include "setupfilehandlertest.moc"