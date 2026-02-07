
#include "RatingDelegate.h"
#include <QPainter>
#include <QApplication>

RatingDelegate::RatingDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void RatingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    // Рисуем фон
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    } else {
        painter->fillRect(opt.rect, opt.palette.base());
    }

    // Получаем рейтинг
    double rating = index.data(Qt::UserRole).toDouble();

    // Рисуем звезды
    int starSize = qMin(opt.rect.height() - 4, 16);
    int starSpacing = 2;
    int totalWidth = 5 * starSize + 4 * starSpacing;
    int startX = opt.rect.left() + (opt.rect.width() - totalWidth) / 2;
    int y = opt.rect.top() + (opt.rect.height() - starSize) / 2;

    QFont font = painter->font();
    font.setPointSize(starSize - 4);
    painter->setFont(font);

    for (int i = 0; i < 5; ++i) {
        int x = startX + i * (starSize + starSpacing);

        if (i < static_cast<int>(rating)) {
            painter->setPen(QColor(255, 204, 0));
            painter->drawText(QRect(x, y, starSize, starSize),
                              Qt::AlignCenter, "★");
        } else {
            painter->setPen(QColor(200, 200, 200));
            painter->drawText(QRect(x, y, starSize, starSize),
                              Qt::AlignCenter, "☆");
        }
    }

    painter->restore();
}

QSize RatingDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(100, 24);
}
