#include "DropTableWidget.h"
#include <QUrl>
#include <qpixmap>

DropTableWidget::DropTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setFocusPolicy(Qt::NoFocus);
}

bool DropTableWidget::isEmptyTable() const
{
    return rowCount() == 0;
}

void DropTableWidget::dragEnterEvent(QDragEnterEvent *event)
{

        if (event->mimeData()->hasUrls()) {
            m_dragActive = true;
            viewport()->update();
            event->acceptProposedAction();
        }


}

void DropTableWidget::dragLeaveEvent(QDragLeaveEvent *event){

    m_dragActive = false;
    viewport()->update();
    event->accept();

}

void DropTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void DropTableWidget::dropEvent(QDropEvent *event)
{
    m_dragActive = false;

    if (event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();

        for (const QUrl &url : urls)
        {
            QString filePath = url.toLocalFile();

            if (!filePath.isEmpty())
                emit filesDropped(filePath);
        }
    }

    viewport()->update();
    event->acceptProposedAction();
}

void DropTableWidget::paintEvent(QPaintEvent *event)
{
    QPixmap m_icon;
    m_icon = QPixmap(":/icons/icons/icons8-drag-and-drop-100.png");

    QTableWidget::paintEvent(event);


    if (!isEmptyTable())
        return;

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = viewport()->rect().adjusted(10, 10, -10, -10);


    QColor bg = m_dragActive ? QColor(0, 120, 215, 40)
                             : QColor(0, 0, 0, 15);
    painter.fillRect(r, bg);


    QPen pen;
    if (m_dragActive) {
        pen.setColor(QColor(0, 120, 215));
        pen.setWidth(2);
    } else {
        pen.setColor(QColor(180, 180, 180));
        pen.setWidth(1);
    }
    pen.setStyle(Qt::DashLine);

    painter.setPen(pen);
    painter.drawRoundedRect(r, 10, 10);

    if (!m_icon.isNull())
    {
        int iconSize = 64;

        QPixmap scaledIcon = m_icon.scaled(
            iconSize, iconSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );

        int x = r.center().x() - scaledIcon.width() / 2;
        int y = r.center().y() - 90;

        if (!m_dragActive)
            painter.setOpacity(0.6);
        else
            painter.setOpacity(1.0);

        painter.drawPixmap(x, y, scaledIcon);

        painter.setOpacity(1.0);
    }


    painter.setPen(QColor(100, 100, 100));

    QFont font = painter.font();
    font.setPointSize(11);
    font.setWeight(QFont::Medium);
    painter.setFont(font);

    QString text = m_dragActive
                       ? "Drop files here"
                       : "Drag & Drop Files Here\nor Click 'Add Files'";

    QRect textRect = r.adjusted(0, 20, 0, 0);

    painter.drawText(textRect, Qt::AlignCenter, text);
}
