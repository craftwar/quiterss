#include <QtGui>

#include <qtsingleapplication.h>
#include "VersionNo.h"
#include "rsslisting.h"

QSplashScreen *splash;

void loadModules(QSplashScreen* psplash)
{
  QElapsedTimer time;
  time.start();

  QProgressBar splashProgress;
  splashProgress.setObjectName("splashProgress");
  splashProgress.setTextVisible(false);
  splashProgress.setFixedHeight(10);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addStretch(1);
  layout->addWidget(&splashProgress);
  splash->setLayout(layout);
  for (int i = 0; i < 100; ) {
    if (time.elapsed() >= 1) {
      time.start();
      ++i;
      qApp->processEvents();
      splashProgress.setValue(i);
      psplash->showMessage("Loading: " + QString::number(i) + "%",
                           Qt::AlignRight | Qt::AlignTop, Qt::white);
    }
  }
}

void createSplashScreen()
{
  splash = 0;
  splash = new QSplashScreen(QPixmap(":/images/images/splashScreen.png"));
  splash->setFixedSize(splash->pixmap().width(), splash->pixmap().height());
  splash->setContentsMargins(15, 0, 15, 0);
  splash->setEnabled(false);
  splash->showMessage("Prepare loading...",
                      Qt::AlignRight | Qt::AlignTop, Qt::white);
  splash->setAttribute(Qt::WA_DeleteOnClose);
  splash->show();
}

int main(int argc, char **argv)
{
  QtSingleApplication app(argc, argv);
  if (app.isRunning()) {
    if (1 == argc) {
      app.sendMessage("--show");
    }
    else {
      QString message = app.arguments().value(1);
      for (int i = 2; i < argc; ++i)
        message += '\n' + app.arguments().value(i);
      app.sendMessage(message);
    }
    return 0;
  }

  app.setApplicationName("QuiteRss");
  app.setOrganizationName("QuiteRss");
  app.setWindowIcon(QIcon(":/images/images/QuiteRSS.ico"));
  app.setQuitOnLastWindowClosed(false);

QString dataDirPath_;
QSettings *settings_;

#if defined(PORTABLE)
  if (PORTABLE) {
    dataDirPath_ = QCoreApplication::applicationDirPath();
    settings_ = new QSettings(
          dataDirPath_ + QDir::separator() + QCoreApplication::applicationName() + ".ini",
          QSettings::IniFormat);
  } else {
    settings_ = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              QCoreApplication::organizationName(), QCoreApplication::applicationName());
    dataDirPath_ = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir d(dataDirPath_);
    d.mkpath(dataDirPath_);
  }
#else
  settings_ = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                            QCoreApplication::organizationName(), QCoreApplication::applicationName());
  dataDirPath_ = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  QDir d(dataDirPath_);
  d.mkpath(dataDirPath_);
#endif

  bool  showSplashScreen_ = settings_->value("Settings/showSplashScreen", true).toBool();

  QString styleActionStr = settings_->value(
        "Settings/styleApplication", "defaultStyle_").toString();
  QString fileString;
  if (styleActionStr == "systemStyle_") {
    fileString = ":/style/systemStyle";
  } else {
    fileString = ":/style/qstyle";
  }
  QFile file(fileString);
  file.open(QFile::ReadOnly);
  app.setStyleSheet(QLatin1String(file.readAll()));

  if (showSplashScreen_)
    createSplashScreen();

  RSSListing rsslisting(settings_, dataDirPath_);

  app.setActivationWindow(&rsslisting, true);
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &rsslisting, SLOT(receiveMessage(const QString&)));

  if (showSplashScreen_)
    loadModules(splash);

  if (!rsslisting.startingTray_ || !rsslisting.showTrayIcon_)
    rsslisting.show();

  if (showSplashScreen_)
    splash->finish(&rsslisting);

  if (rsslisting.reopenFeedStartup_)
    rsslisting.setCurrentFeed();

  if (rsslisting.showTrayIcon_) {
    qApp->processEvents();
    rsslisting.traySystem->show();
  }

  return app.exec();
}
