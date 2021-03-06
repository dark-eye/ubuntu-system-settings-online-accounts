/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This file is part of online-accounts-ui
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAU_SERVICE_H
#define OAU_SERVICE_H

#include <QDBusContext>
#include <QObject>
#include <QVariantMap>

namespace OnlineAccountsUi {

class ServicePrivate;

class Service: public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit Service(QObject *parent = 0);
    ~Service();

public Q_SLOTS:
    QVariantMap requestAccess(const QVariantMap &options);

private:
    ServicePrivate *d_ptr;
    Q_DECLARE_PRIVATE(Service)
};

} // namespace

#endif // OAU_SERVICE_H
