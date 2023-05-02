// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QQuickFramebufferObject>

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include "qthelper.hpp"

class MpvRenderer;

class MpvPlayer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(qreal position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(QSize sourceSize READ sourceSize NOTIFY sourceSizeChanged)

    friend class MpvRenderer;

public:
    static void on_update(void *ctx);

    explicit MpvPlayer(QQuickItem *parent = nullptr);
    ~MpvPlayer() override;

    Renderer *createRenderer() const override;

    qreal position() const;
    qreal duration() const;
    bool paused() const;
    QSize sourceSize() const;

public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void setPosition(double value);
    void seek(qreal offset);
    void command(const QVariant &params);
    void setOption(const QString &name, const QVariant &value);
    void setProperty(const QString &name, const QVariant &value);
    QVariant getProperty(const QString &name);

Q_SIGNALS:
    void positionChanged();
    void durationChanged();
    void pausedChanged();
    void onUpdate();
    void sourceSizeChanged();

private Q_SLOTS:
    void onMpvEvents();
    void doUpdate();

private:
    bool m_paused = true;
    qreal m_position = 0;
    qreal m_duration = 0;
    QSize m_sourceSize;

    mpv_handle *mpv = nullptr;
    mpv_render_context *mpv_gl = nullptr;
};