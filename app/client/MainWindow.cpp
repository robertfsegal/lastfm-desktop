/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSizeGrip>
#include <QTimer>
#include <QMenuBar>
#include <QShortcut>
#include <QToolBar>
#include <QDockWidget>
#include <QScrollArea>

#include "MainWindow.h"

#include "Dialogs/SettingsDialog.h"

#include "Application.h"
#include "Services/RadioService.h"
#include "Services/ScrobbleService.h"
#include "MediaDevices/DeviceScrobbler.h"
#include "../Widgets/ProfileWidget.h"
#include "../Widgets/FriendListWidget.h"
#include "../Widgets/ScrobbleControls.h"
#include "../Widgets/NowPlayingStackedWidget.h"
#include "../Widgets/RecentTracksWidget.h"
#include "../Widgets/SideBar.h"
#include "../Widgets/StatusBar.h"
#include "../Widgets/TitleBar.h"
#include "../Widgets/PlaybackControlsWidget.h"
#include "../Widgets/RadioWidget.h"
#include "../Widgets/NowPlayingWidget.h"
#include "lib/unicorn/widgets/DataBox.h"
#include "lib/unicorn/widgets/MessageBar.h"
#include "lib/unicorn/widgets/GhostWidget.h"
#include "lib/unicorn/widgets/UserToolButton.h"
#include "lib/unicorn/widgets/MessageBar.h"
#include "lib/unicorn/StylableWidget.h"
#include "lib/unicorn/qtwin.h"
#include "lib/unicorn/layouts/SlideOverLayout.h"
#include "lib/listener/PlayerConnection.h"


MainWindow::MainWindow()
{
    hide();

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac( true );
#endif
    
    setCentralWidget(new QWidget);

    QVBoxLayout* layout = new QVBoxLayout( centralWidget() );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    layout->addWidget( ui.messageBar = new MessageBar( this ) );

    QHBoxLayout* h = new QHBoxLayout();
    h->setContentsMargins( 0, 0, 0, 0 );
    h->setSpacing( 0 );

    layout->addLayout( h );

    h->addWidget( ui.sideBar = new SideBar( this ) );

    h->addWidget( ui.stackedWidget = new QStackedWidget( this ) );

    connect( ui.sideBar, SIGNAL(currentChanged(int)), ui.stackedWidget, SLOT(setCurrentIndex(int)));

    ui.stackedWidget->addWidget( ui.nowPlaying = new NowPlayingStackedWidget(this) );
    ui.nowPlaying->setObjectName( "nowPlaying" );

    ui.stackedWidget->addWidget( ui.recentTracks = new RecentTracksWidget( this ) );
    ui.recentTracks->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    connect( ui.stackedWidget, SIGNAL(currentChanged(int)), ui.recentTracks, SLOT(onCurrentChanged(int)) );

    ui.stackedWidget->addWidget( ui.profileScrollArea = new QScrollArea( this ) );
    ui.profileScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui.profileScrollArea->setWidget( ui.profile = new ProfileWidget(this) );
    ui.profileScrollArea->setWidgetResizable( true );
    ui.profile->setObjectName( "profile" );

    ui.stackedWidget->addWidget( ui.friends = new FriendListWidget(this) );
    ui.friends->setObjectName( "friends" );

    ui.stackedWidget->addWidget( ui.radioScrollArea = new QScrollArea( this ) );
    ui.radioScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui.radioScrollArea->setWidget( ui.radio = new RadioWidget( this ) );
    ui.radioScrollArea->setWidgetResizable( true );
    ui.radio->setObjectName( "radio" );

    ui.statusBar = new StatusBar( this );
    ui.statusBar->setObjectName( "StatusBar" );

    ui.statusBar->setSizeGripEnabled( false );

    setWindowTitle( aApp->applicationName() );
    setUnifiedTitleAndToolBarOnMac( true );

    connect( qApp, SIGNAL( sessionChanged( unicorn::Session* ) ), SLOT( onSessionChanged( unicorn::Session* ) ) );
    connect( &ScrobbleService::instance(), SIGNAL( trackStarted(Track, Track) ), SLOT( onTrackStarted(Track, Track) ) );
    connect( &ScrobbleService::instance(), SIGNAL( paused() ), SLOT( onPaused() ) );
    connect( &ScrobbleService::instance(), SIGNAL( resumed() ), SLOT( onResumed() ) );
    connect( &ScrobbleService::instance(), SIGNAL( stopped() ), SLOT( onStopped() ) );

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn()));
    connect( &RadioService::instance(), SIGNAL(error(int,QVariant)), SLOT(onRadioError(int,QVariant)));

    menuBar()->hide();

    //for some reason some of the stylesheet is not being applied properly unless reloaded
    //here. StyleSheets see very flaky to me. :s
    aApp->refreshStyleSheet();

    setMinimumWidth( 540 );

    setStatusBar( ui.statusBar );

    finishUi();

    setupMenuBar();

    resize( 565, 710 );

    show();
}

void
MainWindow::setupMenuBar()
{
    /// File menu (should only show on non-mac)
    QMenu* fileMenu = menuBar()->addMenu( tr( "File" ) );
    QAction* quit = fileMenu->addAction( tr("&Quit"), qApp, SLOT(quit()) );
    quit->setMenuRole( QAction::QuitRole );
#ifdef Q_OS_WIN
    quit->setShortcut( Qt::ALT + Qt::Key_F4 );
#else
    quit->setShortcut( Qt::CTRL + Qt::Key_Q );
#endif

    /// View
    QMenu* viewMenu = menuBar()->addMenu( tr("View") );
    ui.sideBar->addToMenu( *viewMenu );
    viewMenu->addSeparator();
    viewMenu->addAction( "My Last.fm Profile", this, SLOT(onVisitProfile()), Qt::CTRL + Qt::Key_P );

    /// Scrobbles
    QMenu* scrobblesMenu = menuBar()->addMenu( tr("Scrobbles") );
    scrobblesMenu->addAction( "Refresh", ui.recentTracks, SLOT(refresh()), Qt::CTRL + Qt::SHIFT + Qt::Key_R );

    /// Controls
    QMenu* controlsMenu = menuBar()->addMenu( tr("Controls") );
    ui.nowPlaying->nowPlaying()->playbackControls()->addToMenu( *controlsMenu  );

    /// Account
    QMenu* accountMenu = menuBar()->addMenu( tr("Account") );

    /// Tools (should only show on non-mac)
    QMenu* toolsMenu = menuBar()->addMenu( tr("Tools") );
    QAction* c4u = toolsMenu->addAction( tr("Check for Updates"), this, SLOT(checkForUpdates()) );
    c4u->setMenuRole( QAction::ApplicationSpecificRole );
    QAction* prefs = toolsMenu->addAction( tr("Options"), this, SLOT(onPrefsTriggered()) );
    prefs->setMenuRole( QAction::PreferencesRole );

    /// Window
    QMenu* windowMenu = menuBar()->addMenu( tr("Window") );

    /// Help
    QMenu* helpMenu = menuBar()->addMenu( tr("Help") );
    QAction* about = helpMenu->addAction( tr("About"), this, SLOT(about()) );
    about->setMenuRole( QAction::AboutRole );
}

void
MainWindow::onVisitProfile()
{
    QDesktopServices::openUrl( aApp->currentSession()->userInfo().www() );
}

void
MainWindow::onPrefsTriggered()
{
    SettingsDialog* settingsDialog = new SettingsDialog();
    settingsDialog->exec();
}


void
MainWindow::onTuningIn()
{
    /* 0 is the now playing widget in the stack */
    ui.sideBar->click( 0 );
}

void
MainWindow::onTrackStarted( const Track& t, const Track& /*previous*/ )
{
    m_currentTrack = t;

    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2 - %3" ).arg( QApplication::applicationName(), RadioService::instance().station().title(), t.toString() ) );
    else
        setWindowTitle( tr( "%1 - %2" ).arg( QApplication::applicationName(), t.toString() ) );
}


void
MainWindow::onStopped()
{
    m_currentTrack = Track();

    setWindowTitle( QApplication::applicationName() );
}


void
MainWindow::onResumed()
{
    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2 - %3" ).arg( QApplication::applicationName(), RadioService::instance().station().title(), m_currentTrack.toString() ) );
    else
        setWindowTitle( tr( "%1 - %2" ).arg( QApplication::applicationName(), m_currentTrack.toString() ) );
}


void
MainWindow::onPaused()
{
    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2 - Paused - %3" ).arg( QApplication::applicationName(), RadioService::instance().station().title(), m_currentTrack.toString() ) );
    else
        setWindowTitle( tr( "%1 - Paused - %2" ).arg( QApplication::applicationName(), m_currentTrack.toString() ) );
}


void
MainWindow::onRadioError( int error, const QVariant& data )
{
    ui.messageBar->show( tr( "%1: %2" ).arg( data.toString(), QString::number( error ) ), "radio" );
}


void
MainWindow::addWinThumbBarButton( QAction* thumbButtonAction )
{
    m_buttons.append( thumbButtonAction );
}


void
MainWindow::addWinThumbBarButtons( QList<QAction*>& thumbButtonActions )
{
    foreach ( QAction* button, m_buttons )
        thumbButtonActions.append( button );
}
