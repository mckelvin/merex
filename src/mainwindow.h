/*
   @Author:McKelvin
   @Date:2011-11-04
   mainwindow here
   */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <marsyasbackend.h>
#include <playbackthread.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        private slots:
            void on_pb_import_clicked();

        void on_pb_predict_clicked();

        void on_pb_playback_clicked();

        //void on_lv_all_clicked(const QModelIndex &index);

        void on_lv_import_clicked(const QModelIndex &index);

        void on_lv_np_clicked(const QModelIndex &index);

        void on_lv_pp_clicked(const QModelIndex &index);

        void on_lv_nn_clicked(const QModelIndex &index);

        void on_lv_pn_clicked(const QModelIndex &index);

        void predict_slot(int mood);

        void play_double_clicked(const QModelIndex &index);

        void on_pb_output_clicked();

    private:
        Ui::MainWindow *ui;
        QStandardItemModel *model;
        QStandardItemModel *model_mood[4];
        PlaybackThread *playback;
        PlaybackThread *predict;
        QStandardItem *currentPredict;
        QString currentSong;
        MarsyasBackend mb;
        QIcon music_icon;


};
#endif // MAINWINDOW_H
