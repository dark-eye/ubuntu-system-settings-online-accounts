/*
 * Copyright (C) 2013-2015 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This file is part of OnlineAccountsClient.
 *
 * OnlineAccountsClient is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OnlineAccountsClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OnlineAccountsClient.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "online-accounts-ui/globals.h"
#include "onlineaccountsui_interface.h"
#include "setup.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QWindow>
#include <climits>

using namespace OnlineAccountsClient;
using namespace com::ubuntu;

namespace OnlineAccountsClient {

class SetupPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Setup)

public:
    inline SetupPrivate(Setup *setup);
    ~SetupPrivate() {};

    void exec();
    QWindow *clientWindow() const;

private Q_SLOTS:
    void onRequestAccessReply(QDBusPendingCallWatcher *watcher);

private:
    OnlineAccountsUi m_onlineAccountsUi;
    QString m_applicationId;
    QString m_serviceId;
    QString m_serviceTypeId;
    QString m_providerId;
    pid_t m_clientPid;
    mutable Setup *q_ptr;
};

}; // namespace

static QDBusConnection sessionBus() {
    /* When this module is used within the online-accounts-service process, we
     * need to use a separate D-Bus connection otherwise link-local calls won't
     * work (QtDBus does not support delayed replies on local calls). */
    return QCoreApplication::applicationName() == "online-accounts-service" ?
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, "oa-private") :
        QDBusConnection::sessionBus();
}

SetupPrivate::SetupPrivate(Setup *setup):
    QObject(setup),
    m_onlineAccountsUi(OAU_SERVICE_NAME,
                       OAU_OBJECT_PATH,
                       sessionBus()),
    m_clientPid(0),
    q_ptr(setup)
{
    m_onlineAccountsUi.setTimeout(INT_MAX);
}

void SetupPrivate::exec()
{
    QVariantMap options;

    options.insert(OAU_KEY_PID,
                   uint(m_clientPid != 0 ? m_clientPid : getpid()));

    QWindow *window = clientWindow();
    if (window) {
        options.insert(OAU_KEY_WINDOW_ID, window->winId());
        /* TODO: remove this hack once Mir supports window reparenting.
         * Since Mir always return the same window Id for all windows, we use
         * the process ID instead; for the time being this is acceptable since
         * Mir/unity8 don't support more than one window per process.
         * See: https://bugs.launchpad.net/bugs/1153666 */
        if (QGuiApplication::platformName().startsWith("ubuntu")) {
            options.insert(OAU_KEY_WINDOW_ID, uint(getpid()));
        }
    }

    if (!m_applicationId.isEmpty()) {
        options.insert(OAU_KEY_APPLICATION, m_applicationId);
    }

    if (!m_serviceId.isEmpty()) {
        options.insert(OAU_KEY_SERVICE_ID, m_serviceId);
    }

    if (!m_serviceTypeId.isEmpty()) {
        options.insert(OAU_KEY_SERVICE_TYPE, m_serviceTypeId);
    }

    if (!m_providerId.isEmpty()) {
        options.insert(OAU_KEY_PROVIDER, m_providerId);
    }

    QDBusPendingReply<QVariantMap> reply =
        m_onlineAccountsUi.requestAccess(options);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply,
                                                                   this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this,
                     SLOT(onRequestAccessReply(QDBusPendingCallWatcher*)));
}

QWindow *SetupPrivate::clientWindow() const
{
    QWindow *window = QGuiApplication::focusWindow();
    if (window) return window;

    /* Otherwise, just return the first toplevel window; later on, if the need
     * arises, we might add a property to let the client explicitly specify
     * which window to use. */
    return QGuiApplication::topLevelWindows().value(0, 0);
}

void SetupPrivate::onRequestAccessReply(QDBusPendingCallWatcher *watcher)
{
    Q_Q(Setup);

    watcher->deleteLater();

    QVariantMap response;

    QDBusPendingReply<QVariantMap> reply = *watcher;
    if (reply.isError()) {
        qWarning() << "RequestAccess failed:" << reply.error();
        response.insert("errorName", reply.error().name());
    } else {
        // At the moment, we don't have any use for the reply.
        response = reply.value();
    }

    Q_EMIT q->finished(response);
}

/*!
 * \qmltype Setup
 * \inqmlmodule Ubuntu.OnlineAccounts.Client 0.1
 * \ingroup Ubuntu
 *
 * \brief Invoke the Online Accounts panel
 *
 * This object can be used by applications to request the creation of an
 * account. By calling the \l exec() method, the Online Accounts panel will
 * appear and guide the user through the creation of an account. Once done, the
 * \l finished() signal will be emitted. The type of account to be created can
 * be configured by this object's properties.
 *
 * Example:
 *
 * \qml
 * import QtQuick 2.9
 * import Ubuntu.Components 1.3
 * import Ubuntu.OnlineAccounts.Client 0.1
 *
 * Rectangle {
 *     width: 400
 *     height: 300
 *
 *     Button {
 *         anchors.centerIn: parent
 *         text: "Create Facebook account"
 *         onClicked: setup.exec()
 *     }
 *
 *     Setup {
 *         id: setup
 *         applicationId: "MyApp"
 *         providerId: "facebook"
 *     }
 * }
 * \endqml
 */
Setup::Setup(QObject *parent):
    QObject(parent),
    d_ptr(new SetupPrivate(this))
{
}

Setup::~Setup()
{
    delete d_ptr;
}

/*!
 * \qmlproperty string Setup::applicationId
 * Specifies which application is asking for access. The value of this string
 * must be equal to the filename of the XML application file (installed in \c
 * /usr/share/accounts/applications/ or \c
 * ~/.local/share/accounts/applications/) minus the \c .application suffix.
 */
void Setup::setApplicationId(const QString &applicationId)
{
    Q_D(Setup);
    if (applicationId == d->m_applicationId) return;
    d->m_applicationId = applicationId;
    Q_EMIT applicationIdChanged();
}

QString Setup::applicationId() const
{
    Q_D(const Setup);
    return d->m_applicationId;
}

/*!
 * \qmlproperty string Setup::serviceId
 * If set to a valid service ID, the user will be asked to create an Online
 * Account which provides this service.
 */
void Setup::setServiceId(const QString &serviceId)
{
    Q_D(Setup);
    if (serviceId == d->m_serviceId) return;
    d->m_serviceId = serviceId;
    Q_EMIT serviceIdChanged();
}

QString Setup::serviceId() const
{
    Q_D(const Setup);
    return d->m_serviceId;
}

/*!
 * \qmlproperty string Setup::serviceTypeId
 * If set to a valid service type, the user will be asked to create an Online
 * Account which supports this service type.
 */
void Setup::setServiceTypeId(const QString &serviceTypeId)
{
    Q_D(Setup);
    if (serviceTypeId == d->m_serviceTypeId) return;
    d->m_serviceTypeId = serviceTypeId;
    Q_EMIT serviceTypeIdChanged();
}

QString Setup::serviceTypeId() const
{
    Q_D(const Setup);
    return d->m_serviceTypeId;
}

/*!
 * \qmlproperty string Setup::providerId
 * If set to a valid provider, the user will be asked to create an Online
 * Account provided by this entity.
 */
void Setup::setProviderId(const QString &providerId)
{
    Q_D(Setup);
    if (providerId == d->m_providerId) return;
    d->m_providerId = providerId;
    Q_EMIT providerIdChanged();
}

QString Setup::providerId() const
{
    Q_D(const Setup);
    return d->m_providerId;
}

/* At the moment this method is meant to be used by unconfined services, when
 * forwarding a request from their client.
 */
void Setup::setClientPid(pid_t clientPid)
{
    Q_D(Setup);
    d->m_clientPid = clientPid;
}

/*!
 * \qmlmethod void Setup::exec()
 * Launches the Online Accounts panel.
 */
void Setup::exec()
{
    Q_D(Setup);
    d->exec();
}

/*!
 * \qmlsignal Setup::finished()
 * Emitted when the Online Accounts panel has been closed.
 */

#include "setup.moc"
