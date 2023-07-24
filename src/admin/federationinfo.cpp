// SPDX-FileCopyrightText: 2023 Rishi Kumar <rsi.dev17@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "admin/federationinfo.h"

#include <QDateTime>
#include <QJsonObject>

QString FederationInfo::id() const
{
    return m_id;
}

QString FederationInfo::domain() const
{
    return m_domain;
}

QDateTime FederationInfo::createdAt() const
{
    return m_createdAt;
}

QString FederationInfo::severity() const
{
    return m_severity;
}

void FederationInfo::setSeverity(const QString &severity)
{
    if (m_severity == severity) {
        return;
    }
    m_severity = severity;
}

bool FederationInfo::rejectMedia() const
{
    return m_rejectMedia;
}

void FederationInfo::setRejectMedia(const bool &rejectMedia)
{
    if (m_rejectMedia == rejectMedia) {
        return;
    }
    m_rejectMedia = rejectMedia;
}

bool FederationInfo::rejectReports() const
{
    return m_rejectReports;
}

void FederationInfo::setRejectReports(const bool &rejectReports)
{
    if (m_rejectReports == rejectReports) {
        return;
    }
    m_rejectReports = rejectReports;
}

QString FederationInfo::privateComment() const
{
    return m_privateComment;
}

void FederationInfo::setPrivateComment(const QString &privateComment)
{
    if (m_privateComment == privateComment) {
        return;
    }
    m_privateComment = privateComment;
}

QString FederationInfo::publicComment() const
{
    return m_publicComment;
}

void FederationInfo::setPublicComment(const QString &publicComment)
{
    if (m_publicComment == publicComment) {
        return;
    }
    m_publicComment = publicComment;
}

bool FederationInfo::obfuscate() const
{
    return m_obfuscate;
}

void FederationInfo::setObfuscate(const bool &obfuscate)
{
    if (m_obfuscate == obfuscate) {
        return;
    }
    m_obfuscate = obfuscate;
}

FederationInfo FederationInfo::fromSourceData(const QJsonObject &doc)
{
    FederationInfo info;
    info.m_id = doc["id"].toString();
    info.m_domain = doc["domain"].toString();
    info.m_createdAt = QDateTime::fromString(doc["created_at"].toString(), Qt::ISODate).toLocalTime();
    info.m_severity = doc["severity"].toString();
    info.m_rejectMedia = doc["reject_media"].toBool();
    info.m_rejectReports = doc["reject_reports"].toBool();
    info.m_privateComment = doc["private_comment"].toString();
    info.m_publicComment = doc["public_comment"].toString();
    info.m_obfuscate = doc["obfuscate"].toBool();
    return info;
}