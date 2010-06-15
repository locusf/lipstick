/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of mhome.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/
#include <QtTest/QtTest>
#include <MApplicationPage>
#include <QFocusEvent>
#include "ut_switcher.h"
#include "switcher.h"
#include "switcherbutton.h"
#include "switcherview.h"
#include "x11wrapper.h"
#include "mscenemanager_stub.h"
#include "mwindow_stub.h"
#include "homeapplication_stub.h"

static QString gWindowInfoTitle;
static QMap<Atom, QString> gInternedAtoms;
static int gTypeOfXSendEvent;
static Atom gMessageTypeOfXSendEvent;
static int gFormatOfXSendEvent;
static Window gWindowOfXSendEvent;
static Display* gDisplayOfXSendEvent;
static long gMaskOfXSendEvent;
static Bool gPropagateFlagOfXSendEvent;
static long gLongParametersOfXSendEvent[5];
static const unsigned long WINDOW_ID_FOR_WINDOW_TO_FRONT = 666;
static const unsigned long WINDOW_ID_FOR_CLOSE_WINDOW = 777;
Atom X11Wrapper::XInternAtom(Display *, const char *name, Bool)
{
    Atom returnValue = reinterpret_cast<Atom>(name);
    gInternedAtoms[returnValue] = QString(name);
    return returnValue;
}

int X11Wrapper::XSelectInput(Display *, Window, long)
{
    return 0;
}

Status X11Wrapper::XGetWindowAttributes(Display *, Window, XWindowAttributes *)
{
    return 0;
}

int X11Wrapper::XGetWindowProperty(Display *, Window, Atom, long, long, Bool, Atom , Atom *, int *, unsigned long *nitems_return, unsigned long *, unsigned char **prop_return)
{
    *nitems_return = 1;
    *prop_return = (unsigned char*)strdup("1");
    return Success;
}

int X11Wrapper::XFree(void *data)
{
    if (data != NULL) {
        delete [](unsigned char *)data;
    }
    return 0;
}

Status X11Wrapper::XGetWMName(Display *, Window, XTextProperty *textProperty)
{
    std::string::size_type strSize = gWindowInfoTitle.toStdString().length();
    textProperty->value = new unsigned char[strSize + 1];
    strncpy((char *)textProperty->value, gWindowInfoTitle.toStdString().c_str(), strSize + 1);
    return Success;
}

Status X11Wrapper::XGetTextProperty(Display *, Window, XTextProperty *, Atom)
{
    return 0;
}

XWMHints *X11Wrapper::XGetWMHints(Display *, Window)
{
    return 0;
}

int X11Wrapper::XFreePixmap(Display *, Pixmap)
{
    return 0;
}

Pixmap X11Wrapper::XCompositeNameWindowPixmap(Display *, Window)
{
    return 0;
}

Damage X11Wrapper::XDamageCreate(Display *, Drawable, int)
{
    return 0;
}

void X11Wrapper::XDamageDestroy(Display *, Damage)
{

}

int X11Wrapper::XSync(Display *, Bool)
{
    return 0;
}

XErrorHandler X11Wrapper::XSetErrorHandler(XErrorHandler)
{
    return 0;
}

int X11Wrapper::XChangeProperty(Display *, Window, Atom, Atom, int, int, unsigned char *, int)
{
    return 0;
}

Status X11Wrapper::XSendEvent(Display *display, Window,
                              Bool propagate, long mask, XEvent *event)
{
    gTypeOfXSendEvent = event->type;
    gDisplayOfXSendEvent = display;
    gMaskOfXSendEvent = mask;
    gPropagateFlagOfXSendEvent = propagate;
    if (event->type == ClientMessage) {
        gFormatOfXSendEvent = event->xclient.format;
        gMessageTypeOfXSendEvent = event->xclient.message_type;
        gWindowOfXSendEvent = event->xclient.window;
        gLongParametersOfXSendEvent[0] = event->xclient.data.l[0];
        gLongParametersOfXSendEvent[1] = event->xclient.data.l[1];
        if (gInternedAtoms[gMessageTypeOfXSendEvent] ==
            QString("_NET_ACTIVE_WINDOW")) {
            gLongParametersOfXSendEvent[2] = event->xclient.data.l[2];
        }
    }
    return 0;
}

QMap<SwitcherButton *, Window> g_windowButtonMap;
QString g_lastSingleShot;
QString g_singleShotTarget;

// Home stubs
class Home : public MApplicationPage
{
public:
    Home(QGraphicsItem *parent = 0);
};

Home::Home(QGraphicsItem *parent) : MApplicationPage(parent)
{
}


MainWindow *g_testMainWindow;

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

MainWindow *MainWindow::instance(bool)
{
    return g_testMainWindow;
}

// SwitcherButton stubs (used by Switcher)
SwitcherButton::SwitcherButton(const QString &title, MWidget *parent, Window window, WindowInfo::WindowPriority windowPriority) :
    MButton(title, parent, new SwitcherButtonModel),
    priority(windowPriority)
{
    g_windowButtonMap[this] = window;
}

SwitcherButton::~SwitcherButton()
{
    g_windowButtonMap.remove(this);
}

void SwitcherButton::switchToWindow()
{
}

void SwitcherButton::resetState()
{
}

void SwitcherButton::close()
{
}

void SwitcherButton::prepareGeometryChange()
{
    QGraphicsItem::prepareGeometryChange();
}

void SwitcherButton::setGeometry(const QRectF &rect)
{
    return MButton::setGeometry(rect);
}

void SwitcherButton::updateIconGeometry()
{
    Ut_Switcher::iconGeometryUpdated.append(this);
}

WindowInfo::WindowPriority SwitcherButton::windowPriority() const
{
    return priority;
}

void SwitcherButton::setWindowPriority(WindowInfo::WindowPriority windowPriority)
{
    priority = windowPriority;
}

QList<SwitcherButton *> Ut_Switcher::iconGeometryUpdated;

Window SwitcherButton::xWindow()
{
    return g_windowButtonMap[this];
}

QHash<Window, QString> g_windowTitles;

WindowInfo::WindowInfo(Window window) : window_(window)
{
}

WindowInfo::WindowInfo() : title_(QString()), window_(0)
{
}

WindowInfo::~WindowInfo()
{
}

const QString& WindowInfo::title() const
{
    return g_windowTitles[window_];
}

QMap<Window, WindowInfo::WindowPriority > g_windowPriorities;

WindowInfo::WindowPriority WindowInfo::windowPriority() const
{
    return g_windowPriorities[window_];
}

Window WindowInfo::window() const
{
    return window_;
}

QList<Atom> WindowInfo::types() const
{
    return QList<Atom>();
}

QList<Atom> WindowInfo::states() const
{
    return QList<Atom>();
}

bool operator==(const WindowInfo &wi1, const WindowInfo &wi2)
{
    return wi1.window() == wi2.window();
}

bool WindowInfo::updateWindowTitle()
{
    return true;
}

void WindowInfo::updateWindowProperties()
{
}

void QTimer::singleShot(int msec, QObject *receiver, const char *member)
{
    Q_UNUSED(msec);

    if( !g_singleShotTarget.isEmpty() ) {
        QMetaObject::invokeMethod(receiver, g_singleShotTarget.toAscii().data(), Qt::DirectConnection);
    }

    g_lastSingleShot = QString(member);
}

void Ut_Switcher::init()
{
    gInternedAtoms.clear();
    gTypeOfXSendEvent = 0;
    gMessageTypeOfXSendEvent = 0;
    gFormatOfXSendEvent = 0;
    gWindowOfXSendEvent = 0;
    gDisplayOfXSendEvent = NULL;
    gMaskOfXSendEvent = 0;
    gPropagateFlagOfXSendEvent = TRUE;
    for (int i = 0;
         i < sizeof(gLongParametersOfXSendEvent)/sizeof(long); ++i) {
        gLongParametersOfXSendEvent[i] = 0;
    }
    Ut_Switcher::iconGeometryUpdated.clear();

    // Creating a switcher also creates the switcher view
    switcher = new Switcher;
    switcher->setView(new SwitcherView(switcher));

    // Connect widget add/remove signals
    connect(this, SIGNAL(windowListUpdated(const QList<WindowInfo> &)), switcher, SLOT(updateWindowList(const QList<WindowInfo> &)));
    connect(this, SIGNAL(windowTitleChanged(Window, QString)), switcher, SLOT(changeWindowTitle(Window, QString)));
    connect(this, SIGNAL(sizePosChanged(const QSizeF &, const QRectF &, const QPointF &)), switcher, SLOT(viewportSizePosChanged(const QSizeF &, const QRectF &, const QPointF &)));

    connect(this, SIGNAL(windowToFront(Window)),
            switcher, SLOT(windowToFront(Window)));
    connect(this, SIGNAL(closeWindow(Window)),
            switcher, SLOT(closeWindow(Window)));
    g_lastSingleShot = QString();
    g_windowTitles.clear();
    g_windowPriorities.clear();
    g_singleShotTarget = "updateButtons";
}

void Ut_Switcher::cleanup()
{
    // Destroy the switcher (and the associated view)
    delete switcher;
}

void Ut_Switcher::initTestCase()
{
    // MApplications must be created manually these days due to theme system changes
    static int argc = 1;
    static char *app_name = (char *)"./ut_switcher";
    app = new HomeApplication(argc, &app_name);

    g_testMainWindow = new MainWindow();

    mSceneManager = new MSceneManager(NULL, NULL);
    gMWindowStub->stubSetReturnValue("sceneManager", mSceneManager);
}

void Ut_Switcher::cleanupTestCase()
{
    delete mSceneManager;
    // Destroy the MApplication
    delete app;
}

void Ut_Switcher::testConstruction()
{
    QVERIFY(disconnect(app,
                       SIGNAL(windowListUpdated(const QList<WindowInfo> &)),
                       switcher,
                       SLOT(updateWindowList(const QList<WindowInfo> &))));
    connect(app,
            SIGNAL(windowListUpdated(const QList<WindowInfo> &)),
            switcher,
            SLOT(updateWindowList(const QList<WindowInfo> &)));
    QVERIFY(disconnect(app,
                       SIGNAL(windowTitleChanged(Window, QString)),
                       switcher,
                       SLOT(changeWindowTitle(Window, QString))));
    connect(app,
            SIGNAL(windowTitleChanged(Window, QString)),
            switcher,
            SLOT(changeWindowTitle(Window, QString)));
}

static void testXSendEventCommonValues()
{
    QCOMPARE(gTypeOfXSendEvent, ClientMessage);
    QCOMPARE(gDisplayOfXSendEvent, QX11Info::display());
    QVERIFY(!gPropagateFlagOfXSendEvent);
    QCOMPARE(gFormatOfXSendEvent, 32);
}

void Ut_Switcher::testWindowToFront()
{
    emit windowToFront(WINDOW_ID_FOR_WINDOW_TO_FRONT);
    testXSendEventCommonValues();
    QCOMPARE(gInternedAtoms[gMessageTypeOfXSendEvent],
             QString("_NET_ACTIVE_WINDOW"));
    QCOMPARE(gWindowOfXSendEvent, WINDOW_ID_FOR_WINDOW_TO_FRONT);
    QCOMPARE(gMaskOfXSendEvent, StructureNotifyMask);
    QCOMPARE(gLongParametersOfXSendEvent[0], 1L);
    QCOMPARE(gLongParametersOfXSendEvent[1], CurrentTime);
    QCOMPARE(gLongParametersOfXSendEvent[2], 0L);
}

void Ut_Switcher::testCloseWindow()
{
    emit closeWindow(WINDOW_ID_FOR_CLOSE_WINDOW);
    testXSendEventCommonValues();
    QCOMPARE(gInternedAtoms[gMessageTypeOfXSendEvent],
             QString("_NET_CLOSE_WINDOW"));
    QCOMPARE(gWindowOfXSendEvent, WINDOW_ID_FOR_CLOSE_WINDOW);
    QCOMPARE(gMaskOfXSendEvent, SubstructureRedirectMask);
    QCOMPARE(gLongParametersOfXSendEvent[0], CurrentTime);
    QCOMPARE(gLongParametersOfXSendEvent[1], (long)QX11Info::appRootWindow(QX11Info::appScreen()));
}

void Ut_Switcher::testWindowAdding()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three SwitcherButtons are added to the model with the correct names
    for (int i = 0; i < 3; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testWindowRemoving()
{
    // Add three test windows to the window list and the desktop window as the last
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // Remove the second window
    l.removeAt(1);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // There should be two items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 2);

    // See that there are two SwitcherButtons in the model with the correct names
    for (int i = 0; i < 2; i++) {
        // The button titles should match the window names (Test0, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i == 0 ? 0 : 2);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testWindowTitleChangeWhenWindowListIsUpdated()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // Change the name of the second window
    g_windowTitles[1] = QString("Test3");

    // Bring home to foreground and update the window list
    emit windowListUpdated(l);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three SwitcherButtons are added to the model with the correct names
    for (int i = 0; i < 3; i++) {
        // The button titles should match the window names (Test0, Test3, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i == 1 ? 3 : i);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testWindowTitleChange()
{
    // Add three test windows to the window list
    QList<WindowInfo> windows = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(windows);

    emit windowTitleChanged(windows[0].window(), QString("Test3"));

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three SwitcherButtons are added to the model with the correct names
    for (int i = 1; i < 3; i++) {
        // The button titles should match the window names (Test0, Test3, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i == 0 ? 3 : i);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testPanning()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    Ut_Switcher::iconGeometryUpdated.clear();

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // Let the Switcher know about a change in panning
    QSizeF viewportSize;
    QRectF pannedRange;
    QPointF pannedPos;
    emit sizePosChanged(viewportSize, pannedRange, pannedPos);

    // The icon geometry of the three windows should be changed
    QCOMPARE(iconGeometryUpdated.count(), 3);
    for (int i = 0; i < 3; i++) {
        QCOMPARE(l.at(i).window(), iconGeometryUpdated.at(i)->xWindow());
    }
}

QList<WindowInfo> Ut_Switcher::createWindowList(int numWindows)
{
    QList<WindowInfo> l;

    for (int i = 0; i < numWindows; i++) {
        g_windowTitles[i] = QString().sprintf("Test%d", i);

        l.append(WindowInfo(i));
    }

    return l;
}


void Ut_Switcher::testWindowOrder()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three SwitcherButtons are added to the model with the correct names
    for (int i = 0; i < 3; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i);
        QCOMPARE(b->text(), title);
    }

    // Create a new list and shuffle its order
    QList<WindowInfo> sl = createWindowList(3);
    sl.swap(1, 2);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(sl);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three the SwitcherButtons are still in the same order
    for (int i = 0; i < 3; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i);
        QCOMPARE(b->text(), title);
        // Check that the window ids are in the same order as in the original list
        QCOMPARE(b->xWindow(), l.at(i).window());
    }
}

void Ut_Switcher::testCallWindowAdding()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // See that three SwitcherButtons are added to the model with the correct names
    for (int i = 0; i < 3; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i);
        QCOMPARE(b->text(), title);
    }

    QString callTitle("Call");

    // Add a new call window
    g_windowTitles[111] = callTitle;
    g_windowPriorities[111] = WindowInfo::Call;
    l.insert(1, WindowInfo(111));

    emit windowListUpdated(l);

    // There should be four items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 4);

    // The call window should be the first one
    QCOMPARE(switcher->model()->buttons().at(0)->text(), QString("Call"));

    // See that three previous the SwitcherButtons are still in the same order
    for (int i = 1; i < 4; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i - 1);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testCallWindowFromExisistingWindow()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    g_windowPriorities[2] = WindowInfo::Call;

    emit windowListUpdated(l);

    // There should be three items in the switcher model
    QCOMPARE(switcher->model()->buttons().count(), 3);

    // The last window should now be the first one
    QCOMPARE(switcher->model()->buttons().at(0)->text(), QString("Test2"));

    // See that the rest of the SwitcherButtons are in the right order
    for (int i = 1; i < 3; i++) {
        // The button titles should match the window names (Test0, Test1, Test2)
        SwitcherButton *b = switcher->model()->buttons().at(i).data();
        QString title = QString().sprintf("Test%d", i - 1);
        QCOMPARE(b->text(), title);
    }
}

void Ut_Switcher::testUpdateDelay()
{
    // Add three test windows to the window list
    QList<WindowInfo> l = createWindowList(3);

    // Prevent immediate update
    g_singleShotTarget.clear();

    // Let the Switcher know about the updated window list
    emit windowListUpdated(l);

    // There should be no items in the switcher model yet
    QCOMPARE(switcher->model()->buttons().count(), 0);

    // But the update should have been scheduled
    QVERIFY(g_lastSingleShot.contains("updateButtons"));
}

QTEST_APPLESS_MAIN(Ut_Switcher)
