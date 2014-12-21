//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UPDATEFORM_H
#define UPDATEFORM_H

#include "definitions.hpp"

#include <QUrl>
#include <QString>
#include <QDialog>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTimer>

class QDomElement;
class QFile;
namespace Ui
{
    class UpdateForm;
}

namespace Huggle
{
    class WebserverQuery;
    class Instruction;

    /*!
     * \brief Update form is shown when there is an update for huggle
     * This form may work on some platforms only
     * 
     * How it works:
     * This updater will download a XML from our huggle server every startup that
     * contains the information what to do in order to update to latest huggle
     * version.
     *
     * Following commands are available so far:
     * <exec>Path</exec> - will execute as regular user
     * <exec elevated=true>Path</exec> - will execute as Administrator on Windows, or root on Linux
     * <copy md5=hash elevated=bool recursive=bool from=path to=path overwrite=bool></copy> - will copy a file
     * <move elevated=bool from=path to=path overwrite=bool></move> - will move a file or folder
     * <folder>path</folder> - will create a folder
     * <remove elevated=bool recursive=bool>path</remove>
     * <download to=path ssl=bool md5=string>URL</download>
     * These commands are only used if following is present
     * <obsolete>version</obsolete> - tell system there is a new version
     * This MUST not be present otherwise the commands will be skipped
     * <info>message</info> - send a message explaining how to update
     * These variables can be used:
     * $root - where huggle is installed (C:\Program Files\Huggle\)
     * $temp - temporary folder
     * $root_bck - backup of root
     */
    class HUGGLE_EX UpdateForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit UpdateForm(QWidget *parent = nullptr);
            void Check();
            ~UpdateForm();
            WebserverQuery *qData;
            QString Error;
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void OnTick();
            void Exit();
            void on_label_linkActivated(const QString &link);
            void httpReadyRead();
            void httpDownloadFinished();
            void updateDownloadProgress(qint64 value, qint64 max);
        private:
            bool parse_xml(QDomElement *line);
            void reject();
            void PreparationFinish();
            void Fail(QString reason);
            //! Pointer to current instruction if this is
            //! nullptr updater will jump on next
            Instruction* inst = nullptr;
            bool ProcessManifest(QString data);
            void ProcessDownload();
            void Write(QString message);
            QString MD5(QString file);
            void NextInstruction();
            void Update();
            void Generic_Exec(QString path, QStringList parameters);
            bool MovingFiles = false;
            int CurrentFile = 0;
            QStringList values;
            QStringList dirs;
            QString Path(QString text);
            QString RootPath;
            QString TempPath;
            QString Manifest;
            Ui::UpdateForm *ui;
            QUrl url;
            QNetworkAccessManager *manager = nullptr;
            QNetworkReply *reply = nullptr;
            QFile *file = nullptr;
            qint64 fileSize;
            QList<Instruction*> Instructions;
            QTimer *timer;
            QUrl *manualDownloadpage = nullptr;
    };

    enum InstructionType
    {
        Instruction_Copy,
        Instruction_Move,
        Instruction_Download,
        Instruction_Delete,
        Instruction_Execute,
        Instruction_Folder
    };

    class Instruction
    {
        public:
            Instruction(InstructionType type, QString from, QString to, bool is_elevated = false, bool is_recursive = false, bool merge = false, bool is_overwriting = false, QString md5 = "");
            InstructionType Type;
            QString Source;
            QString Destination;
            QString Hash_MD5;
            QString Original;
            bool Elevated;
            bool Overwrite;
            bool Is_Recursive;
            bool Is_Merging;
            int operation = 0;
    };
}
#endif // UPDATEFORM_H
