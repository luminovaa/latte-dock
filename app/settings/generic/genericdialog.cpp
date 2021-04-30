/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "genericdialog.h"

// Qt
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QVBoxLayout>

// KDE
#include <KLocalizedString>
#include <KStandardGuiItem>

static const int ERRORINTERVAL = 4000;
static const int INFORMATIONINTERVAL = 3000;
static const int INFORMATIONWITHACTIONINTERVAL = 5000;
static const int POSITIVEINTERVAL = 3000;
static const int WARNINGINTERVAL = 3500;

namespace Latte {
namespace Settings {
namespace Dialog {

GenericDialog::GenericDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
}

GenericDialog::~GenericDialog()
{
}

KMessageWidget *GenericDialog::initMessageWidget()
{
    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout *>(layout());

    if (!vLayout) {
        return nullptr;
    }

    auto messagewidget = new KMessageWidget(this);
    vLayout->insertWidget(vLayout->count()-1, messagewidget);
    connect(messagewidget, &KMessageWidget::hideAnimationFinished, messagewidget, &QObject::deleteLater);

    return messagewidget;
}

KMessageBox::ButtonCode GenericDialog::saveChangesConfirmation(const QString &text)
{
    QString dialogtext = text.isEmpty() ? i18n("The settings have changed.<br/>Do you want to apply the changes or discard them?") : text;

    return KMessageBox::warningYesNoCancel(this,
                                           dialogtext,
                                           i18n("Apply Settings"),
                                           KStandardGuiItem::apply(),
                                           KStandardGuiItem::discard());
}

void GenericDialog::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent, QList<QAction *> actions)
{
    if (msg.isEmpty()) {
        return;
    }

    auto messagewidget = initMessageWidget();

    if (!messagewidget) {
        return;
    }

    int hideInterval = 0;

    if (!isPersistent) {
        if (type == KMessageWidget::Error) {
            hideInterval = ERRORINTERVAL;
        } else if (type == KMessageWidget::Warning) {
            hideInterval = WARNINGINTERVAL;
        } else if (type == KMessageWidget::Information) {
            hideInterval = (actions.count() == 0 ?  INFORMATIONINTERVAL : INFORMATIONWITHACTIONINTERVAL);
        } else if (type == KMessageWidget::Positive) {
        }
    }

    messagewidget->setCloseButtonVisible(!isPersistent || actions.count()==0);

    for (int i=0; i<actions.count(); ++i) {
        connect(actions[i], &QAction::triggered, messagewidget, &KMessageWidget::animatedHide);
        messagewidget->addAction(actions[i]);
    }

    messagewidget->setText(msg);

    // TODO: wrap at arbitrary character positions once QLabel can do this
    // https://bugreports.qt.io/browse/QTBUG-1276
    messagewidget->setWordWrap(true);
    messagewidget->setMessageType(type);
    messagewidget->setWordWrap(false);

    const int unwrappedWidth = messagewidget->sizeHint().width();
    messagewidget->setWordWrap(unwrappedWidth > size().width());

    // for some reason this is not smoooth so it is disabled
    //messagewidget->animatedShow();

    if (hideInterval > 0) {
        QTimer *hidetimer = new QTimer(messagewidget);
        hidetimer->setInterval(hideInterval);

        connect(hidetimer, &QTimer::timeout, this, [&, messagewidget]() {
                messagewidget->animatedHide();
        });

        hidetimer->start();
    }
}

}
}
}
