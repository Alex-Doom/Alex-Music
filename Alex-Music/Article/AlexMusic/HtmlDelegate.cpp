// HtmlDelegate.cpp
#include "HtmlDelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QMouseEvent>
#include <QDebug>
#include "MainWindow.h" // Добавляем этот include

HtmlDelegate::HtmlDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void HtmlDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    // Проверяем, содержит ли элемент HTML
    QString text = index.data(Qt::DisplayRole).toString();
    bool isHtml = text.contains("<") && text.contains(">");

    if (!isHtml) {
        // Обычный текст - используем стандартный делегат
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(options.font);
    doc.setTextWidth(options.rect.width());

    // Отрисовываем фон
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // Отрисовываем текст
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

// Реализация RatingTableDelegate
RatingTableDelegate::RatingTableDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void RatingTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
    // Используем статический метод MainWindow для получения COL_RATING
    int ratingColumn = MainWindow::ratingColumn();

    if (index.column() == ratingColumn) {
        // Отрисовываем фон
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        // Получаем рейтинг
        QVariant ratingVariant = index.data(Qt::UserRole + 2);
        double rating = ratingVariant.isValid() ? ratingVariant.toDouble() : 0.0;
        int fullStars = static_cast<int>(rating);

        // Рисуем звезды
        QRect rect = option.rect;
        int starSize = qMin(rect.height() - 4, 20);
        int spacing = 2;
        int totalWidth = 5 * (starSize + spacing) - spacing;
        int startX = rect.left() + (rect.width() - totalWidth) / 2;

        painter->save();

        // Устанавливаем цвет текста
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.text().color());
        }

        QFont font = painter->font();
        font.setPointSize(starSize - 5);
        painter->setFont(font);

        for (int i = 0; i < 5; i++) {
            QRect starRect(startX + i * (starSize + spacing),
                           rect.top() + (rect.height() - starSize) / 2,
                           starSize, starSize);

            if (i < fullStars) {
                // Желтые звезды для рейтинга
                painter->setPen(QColor(255, 204, 0)); // Золотой цвет
                painter->drawText(starRect, Qt::AlignCenter, "★");
            } else {
                // Серые звезды
                painter->setPen(QColor(150, 150, 150));
                painter->drawText(starRect, Qt::AlignCenter, "☆");
            }
        }

        painter->restore();
    } else {
        // Для других колонок используем стандартную отрисовку
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool RatingTableDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) {
    int ratingColumn = MainWindow::ratingColumn();

    if (index.column() == ratingColumn && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QRect rect = option.rect;

        int starSize = qMin(rect.height() - 4, 20);
        int spacing = 2;
        int totalWidth = 5 * (starSize + spacing) - spacing;
        int startX = rect.left() + (rect.width() - totalWidth) / 2;

        // Определяем, на какую звезду кликнули
        for (int i = 0; i < 5; i++) {
            QRect starRect(startX + i * (starSize + spacing),
                           rect.top() + (rect.height() - starSize) / 2,
                           starSize, starSize);

            if (starRect.contains(mouseEvent->pos())) {
                // Устанавливаем новый рейтинг
                double newRating = i + 1.0;
                model->setData(index, newRating, Qt::UserRole + 2);

                // Создаем строку звезд для отображения
                QString ratingText;
                for (int s = 0; s < 5; ++s) {
                    ratingText += (s < newRating) ? "★" : "☆";
                }
                model->setData(index, ratingText, Qt::DisplayRole);

                // Сигнализируем об изменении
                emit ratingChanged(index.row(), static_cast<int>(newRating));
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
