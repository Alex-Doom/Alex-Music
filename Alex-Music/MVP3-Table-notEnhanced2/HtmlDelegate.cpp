#include "HtmlDelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QMouseEvent>
#include <QDebug>

HtmlDelegate::HtmlDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void HtmlDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QString text = index.data(Qt::DisplayRole).toString();
    bool isHtml = text.contains("<") && text.contains(">");

    if (!isHtml) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(options.font);
    doc.setTextWidth(options.rect.width());

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    QRect textRect = options.rect.adjusted(2, 2, -2, -2);
    painter->translate(textRect.topLeft());
    doc.drawContents(painter, QRect(0, 0, textRect.width(), textRect.height()));
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    bool isHtml = text.contains("<") && text.contains(">");

    if (!isHtml) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(option.font);
    doc.setTextWidth(option.rect.width());

    return QSize(doc.idealWidth(), doc.size().height());
}

// RatingTableDelegate implementation
RatingTableDelegate::RatingTableDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void RatingTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
    // Отрисовываем фон
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // Получаем рейтинг
    double rating = index.data(Qt::UserRole + 2).toDouble();
    int fullStars = static_cast<int>(rating);

    QRect rect = option.rect;
    int starSize = qMin(rect.height() - 4, 20);
    int spacing = 2;
    int totalWidth = 5 * (starSize + spacing) - spacing;
    int startX = rect.left() + (rect.width() - totalWidth) / 2;

    painter->save();
    QFont font = painter->font();
    font.setPointSize(starSize - 5);
    painter->setFont(font);

    for (int i = 0; i < 5; i++) {
        QRect starRect(startX + i * (starSize + spacing),
                       rect.top() + (rect.height() - starSize) / 2,
                       starSize, starSize);

        if (i < fullStars) {
            painter->setPen(QColor(255, 204, 0));
            painter->drawText(starRect, Qt::AlignCenter, "★");
        } else {
            painter->setPen(QColor(150, 150, 150));
            painter->drawText(starRect, Qt::AlignCenter, "☆");
        }
    }
    painter->restore();
}

bool RatingTableDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QRect rect = option.rect;

        int starSize = qMin(rect.height() - 4, 20);
        int spacing = 2;
        int totalWidth = 5 * (starSize + spacing) - spacing;
        int startX = rect.left() + (rect.width() - totalWidth) / 2;

        for (int i = 0; i < 5; i++) {
            QRect starRect(startX + i * (starSize + spacing),
                           rect.top() + (rect.height() - starSize) / 2,
                           starSize, starSize);

            if (starRect.contains(mouseEvent->pos())) {
                double newRating = i + 1.0;
                model->setData(index, newRating, Qt::UserRole + 2);

                QString ratingText;
                for (int s = 0; s < 5; ++s) {
                    ratingText += (s < newRating) ? "★" : "☆";
                }
                model->setData(index, ratingText, Qt::DisplayRole);

                emit ratingChanged(index.row(), static_cast<int>(newRating));
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
