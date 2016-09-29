/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016  Franco Martelli

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "configuration.h"
#include <fstream>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QDirIterator>


XDGSearch::Configuration::Configuration(const XDGSearch::Pool& p = Pool::END)
{
    switch(p)       {
            case Pool::DESKTOP :
        d = std::unique_ptr<ConfigurationBase>(new cfgDesktop);
        d ->defaultSettings("XDG_DESKTOP_DIR");
        break;
            case Pool::TEMPLATES :
        d = std::unique_ptr<ConfigurationBase>(new cfgTemplates);
        d ->defaultSettings("XDG_TEMPLATES_DIR");
        break;
            case Pool::PUBLICSHARE :
        d = std::unique_ptr<ConfigurationBase>(new cfgPublicShare);
        d ->defaultSettings("XDG_PUBLICSHARE_DIR");
        break;
            case Pool::DOCUMENTS :
        d = std::unique_ptr<ConfigurationBase>(new cfgDocuments);
        d ->defaultSettings("XDG_DOCUMENTS_DIR");
        break;
            case Pool::MUSIC :
        d = std::unique_ptr<ConfigurationBase>(new cfgMusic);
        d ->defaultSettings("XDG_MUSIC_DIR");
        break;
            case Pool::PICTURES :
        d = std::unique_ptr<ConfigurationBase>(new cfgPictures);
        d ->defaultSettings("XDG_PICTURES_DIR");
        break;
            case Pool::VIDEOS :
        d = std::unique_ptr<ConfigurationBase>(new cfgVideos);
        d ->defaultSettings("XDG_VIDEOS_DIR");
        break;
            case Pool::SOURCES :
        d = std::unique_ptr<ConfigurationBase>(new cfgSources);
        d ->defaultSettings("XDG_SOURCES_DIR");
        break;
            case Pool::END :
        d = nullptr;
        break;
            default :
        d = nullptr;
    }
}

/*     const std::list<QString> xdgkeywordslist = {  "XDG_DESKTOP_DIR"     // folder.png
 *                                                 XDG_DOWNLOAD_DIR
                                               ,  "XDG_TEMPLATES_DIR"   // presentation.png
                                               ,  "XDG_PUBLICSHARE_DIR" // html.png
                                               ,  "XDG_DOCUMENTS_DIR"   // document.png
                                               ,  "XDG_MUSIC_DIR"       // sownd.png
                                               ,  "XDG_PICTURES_DIR"    // image.png
                                               ,  "XDG_VIDEOS_DIR"};    // video.png
                                                */

XDGSearch::Pool& XDGSearch::operator++(Pool& p)
{
        switch(p)       {
                case Pool::DESKTOP :     return p = Pool::TEMPLATES;
                case Pool::TEMPLATES :   return p = Pool::PUBLICSHARE;
                case Pool::PUBLICSHARE : return p = Pool::DOCUMENTS;
                case Pool::DOCUMENTS :   return p = Pool::MUSIC;
                case Pool::MUSIC :       return p = Pool::PICTURES;
                case Pool::PICTURES :    return p = Pool::VIDEOS;
                case Pool::VIDEOS :      return p = Pool::SOURCES;
                case Pool::SOURCES :     return p = Pool::END;
                case Pool::END :         return p = Pool::END;
                default :                return p = Pool::END;
        }
}

void XDGSearch::cfgDesktop::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Desktop";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("odt");
    addHelperToPool("image");
}

void XDGSearch::cfgTemplates::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Templates";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("odt");
    addHelperToPool("text");
}

void XDGSearch::cfgPublicShare::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "PublicShare";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("odt");
    addHelperToPool("image");
    addHelperToPool("pdf");
}

void XDGSearch::cfgDocuments::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Documents";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("odt");
    addHelperToPool("pdf");
}

void XDGSearch::cfgMusic::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Music";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("music");
}

void XDGSearch::cfgPictures::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Pictures";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("image");
}

void XDGSearch::cfgVideos::defaultSettings(const std::string& XDGKeyword)
{
    auto p = getXDGKeysDirPath(XDGKeyword);
    auto emptyP = std::make_pair(std::string(), std::string());
    std::get<0>(pools) = XDGKeyword;
    if(p != emptyP) {
        std::get<1>(pools) = p.first;
        std::get<3>(pools) = p.second;
    }
    else {
        std::get<1>(pools) = "Videos";
    }
    std::get<4>(pools) = "none";
    addHelperToPool("media");
}

void XDGSearch::cfgSources::defaultSettings(const std::string& XDGKeyword)
{
    std::get<0>(pools) = XDGKeyword;
    std::get<1>(pools) = "Sources";
    std::get<4>(pools) = "none";

    addHelperToPool("text");
}

void XDGSearch::Settings::defaultSettings(const std::string& n = std::string())
{
    Q_UNUSED(n);

    settings.beginGroup("helpers");
    auto keys = settings.childKeys();
    settings.remove("");
    settings.endGroup();

    for(const auto& k : keys)
        settings.remove(k);

    settings.beginGroup("global");
    settings.remove("");
    settings.endGroup();

    initSettings();

}

const std::pair<std::string, std::string> XDGSearch::ConfigurationBase::getXDGKeysDirPath(const std::string& s)
{
    std::pair<std::string, std::string> xdgDirectories;
    QString home = QDir::homePath()
          , xdgFile = home + "/.config/user-dirs.dirs";

    std::ifstream ifs(xdgFile.toStdString());

    if(ifs)
    {
        const QSettings xdguserdir(xdgFile, QSettings::NativeFormat);
        xdgDirectories.first = xdguserdir.value(QString::fromStdString(s), "").toString().replace(0, 6, "").toStdString();
        xdgDirectories.second = xdguserdir.value(QString::fromStdString(s), "").toString().replace(0, 5, home).toStdString();
    }
    ifs.close();
    return xdgDirectories;

/*  Can't use QStandardPaths, see: https://bugreports.qt.io/browse/QTBUG-36171
    if(ifs)
    {
        std::cout << "xdg" << std::endl;
        xdgDirectories.push_back(QStandardPaths::displayName(QStandardPaths::DesktopLocation).toStdString());
        xdgDirectories.push_back(QStandardPaths::displayName(QStandardPaths::DocumentsLocation).toStdString());
        xdgDirectories.push_back(QStandardPaths::displayName(QStandardPaths::MusicLocation).toStdString());
        xdgDirectories.push_back(QStandardPaths::displayName(QStandardPaths::PicturesLocation).toStdString());
    }*/
}

bool XDGSearch::ConfigurationBase::isFirstRun()
{
    auto workingDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir appDBDir;
    appDBDir.mkpath(workingDir);
    appDBDir.setCurrent(workingDir);    // set application working directory
    if(appDBDir.mkdir("stopwords"))  {  // true if not exist, make directory and populate merging stopwords files
        QDirIterator dirIt("/usr/share/xapian-core/stopwords", QDir::Files);

        while(dirIt.hasNext())  {
            const std::string  fileFullPathName = dirIt.next().toStdString()
                             , cmd = "cp "
                                   + fileFullPathName
                                   + " stopwords";
            std::system(cmd.c_str());
        }
    }
    settings.beginGroup("global");      // test is first run checking askQuitConfirmation key in xdgsearch.conf
    bool retval = settings.contains("askQuitConfirmation");
    settings.endGroup();
    return !retval;
}

void XDGSearch::ConfigurationBase::writeSettings(const helperType& ht)
{
    settings.beginGroup("helpers");
    settings.setValue(QString::fromStdString(std::get<0>(ht)), true);
    settings.endGroup();

    settings.beginGroup(QString::fromStdString(std::get<0>(ht)));
    settings.setValue("extension", QString::fromStdString(std::get<1>(ht)));
    settings.setValue("commandline", QString::fromStdString(std::get<2>(ht)));
    settings.setValue("granularity", std::get<3>(ht));
    settings.endGroup();
}

void XDGSearch::ConfigurationBase::writeSettings(const poolType& pt)
{
    settings.beginGroup(QString::fromStdString(std::get<0>(pt)));
    settings.setValue("localpoolname", QString::fromStdString(std::get<1>(pt)));
    settings.setValue("poolhelpers", QString::fromStdString(std::get<2>(pt)));
    settings.setValue("pooldirpath", QString::fromStdString(std::get<3>(pt)));
    settings.setValue("stemmed", QString::fromStdString(std::get<4>(pt)));
    settings.setValue("stopwordsfile", QString::fromStdString(std::get<5>(pt)));
    settings.endGroup();
}

void XDGSearch::ConfigurationBase::initSettings()
{
    writeSettings(std::make_tuple("text","txt,cpp,h","/bin/cat",6));
    writeSettings(std::make_tuple("pdf","pdf","/usr/bin/pdftotext",6));
    writeSettings(std::make_tuple("odt","odt","/usr/bin/odt2txt",6));
    writeSettings(std::make_tuple("image","jpg,jpeg,png","/usr/bin/iinfo -v",0));
    writeSettings(std::make_tuple("music","mp3,ogg","/usr/bin/mediainfo",0));
    writeSettings(std::make_tuple("video","mpg,avi,webm","/usr/bin/mediainfo",0));

    settings.beginGroup("global");
    settings.setValue("askQuitConfirmation", false);
    settings.endGroup();
}

const XDGSearch::helperType XDGSearch::ConfigurationBase::enqueryHelper(const std::string& h)
{
    helpers = std::make_tuple("","","",0);
    settings.beginGroup("helpers");
    bool val = settings.value(QString::fromStdString(h)).toBool();
    if(val)
    {
        settings.endGroup();
        std::get<0>(helpers) = h;
        settings.beginGroup(QString::fromStdString(h));
        std::get<1>(helpers) = settings.value("extension").toString().toStdString();
        std::get<2>(helpers) = settings.value("commandline").toString().toStdString();
        std::get<3>(helpers) = settings.value("granularity").toInt();
    }
    settings.endGroup();
    return helpers;
}

const XDGSearch::poolType XDGSearch::ConfigurationBase::enqueryPool()
{
    pools = std::make_tuple(std::get<0>(pools),"","","","","");
    settings.beginGroup(QString::fromStdString(std::get<0>(pools)));
    bool b = settings.childKeys().isEmpty();
    if(!b) {
        std::get<1>(pools) = settings.value("localpoolname").toString().toStdString();
        std::get<2>(pools) = settings.value("poolhelpers").toString().toStdString();
        std::get<3>(pools) = settings.value("pooldirpath").toString().toStdString();
        std::get<4>(pools) = settings.value("stemmed").toString().toStdString();
        std::get<5>(pools) = settings.value("stopwordsfile").toString().toStdString();
    }
    settings.endGroup();
    return pools;
}

void XDGSearch::ConfigurationBase::removeHelper(const std::string& h)
{
    settings.beginGroup("helpers");
    settings.remove(QString::fromStdString(h));
    settings.endGroup();

    settings.remove(QString::fromStdString(h));
}

bool XDGSearch::ConfigurationBase::askForConfirmation()
{
    settings.beginGroup("global");
    bool retval = settings.value("askQuitConfirmation", false).toBool();
    settings.endGroup();
    return retval;
}

void XDGSearch::ConfigurationBase::saveMainWindowGeometry(const QByteArray& g)
{
    settings.beginGroup("global");
    settings.setValue("geometry", g);
    settings.endGroup();
}

const QByteArray XDGSearch::ConfigurationBase::readMainWindowGeometry()
{
    settings.beginGroup("global");
    const auto retval = settings.value("geometry", QByteArray()).toByteArray();
    settings.endGroup();
    return retval;
}

void XDGSearch::ConfigurationBase::setAskForConfirmation(bool b)
{
    settings.beginGroup("global");
    settings.setValue("askQuitConfirmation", b);
    settings.endGroup();
}

bool XDGSearch::ConfigurationBase::isPopulatedDB(const Pool& p)
{
    const std::string k = toXDGKey(p);
    settings.beginGroup(QString::fromStdString(k));
    const auto dbName = settings.value("localpoolname").toString();
    settings.endGroup();
    QDir d(dbName);

    return d.exists();
}

std::string XDGSearch::toXDGKey(const XDGSearch::Pool& p)
{
    std::string retval;
    switch(p)       {
            case Pool::DESKTOP :
        retval = "XDG_DESKTOP_DIR";
        break;
            case Pool::TEMPLATES :
        retval = "XDG_TEMPLATES_DIR";
        break;
            case Pool::PUBLICSHARE :
        retval = "XDG_PUBLICSHARE_DIR";
        break;
            case Pool::DOCUMENTS :
        retval = "XDG_DOCUMENTS_DIR";
        break;
            case Pool::MUSIC :
        retval = "XDG_MUSIC_DIR";
        break;
            case Pool::PICTURES :
        retval = "XDG_PICTURES_DIR";
        break;
            case Pool::VIDEOS :
        retval = "XDG_VIDEOS_DIR";
        break;
            case Pool::SOURCES :
        retval = "XDG_SOURCES_DIR";
        break;
            case Pool::END :
        break;
            default :
        ;
    }
    return retval;
}

QStringList XDGSearch::ConfigurationBase::getHelpersNameList()
{
    settings.beginGroup("helpers");
    auto retval = settings.childKeys();
    settings.endGroup();

    return retval;
}
