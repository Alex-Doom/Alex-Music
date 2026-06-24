#include "Delegates.h"
#include <QPainter>
#include <QTextDocument>
#include <QMouseEvent>
#include <QApplication>

HtmlDelegate::HtmlDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void HtmlDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    if (!text.contains('<')) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(opt.font);
    doc.setTextWidth(opt.rect.width());

    painter->translate(opt.rect.left() + 2, opt.rect.top() + 2);
    doc.drawContents(painter, QRect(0, 0, opt.rect.width() - 4, opt.rect.height() - 4));
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    if (!text.contains('<')) return QStyledItemDelegate::sizeHint(option, index);

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(option.font);
    return QSize(static_cast<int>(doc.idealWidth()), static_cast<int>(doc.size().height()));
}

RatingDelegate::RatingDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void RatingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }

    double rating = index.data(Qt::UserRole + 1).toDouble();
    int fullStars = static_cast<int>(rating);

    QRect rect = opt.rect;
    int starSize = qMin(rect.height() - 4, 16);
    int x = rect.left() + (rect.width() - 5 * starSize) / 2;
    int y = rect.top() + (rect.height() - starSize) / 2;

    painter->save();
    for (int i = 0; i < 5; ++i) {
        painter->setPen(i < fullStars ? QColor(255, 204, 0) : QColor(150, 150, 150));
        painter->drawText(QRect(x + i * starSize, y, starSize, starSize),
                          Qt::AlignCenter, i < fullStars ? "★" : "☆");
    }
    painter->restore();
}

bool RatingDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                 const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(event);
        QRect rect = option.rect;
        int starSize = qMin(rect.height() - 4, 16);
        int startX = rect.left() + (rect.width() - 5 * starSize) / 2;

        int star = (me->position().x() - startX) / starSize;
        if (star >= 0 && star < 5) {
            model->setData(index, star + 1.0, Qt::UserRole + 1);
            emit ratingChanged(index.row(), star + 1);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
