/*
   @Author:McKelvin
   @Date:2011-11-01
   */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#define msg(x) QMessageBox::information(this,tr("Message"),x);
using namespace Marsyas;


QString LAST_IMPORT_DIR="/media/Media/mirex_music";
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new QStandardItemModel();
    model_mood[0] = new QStandardItemModel();
    model_mood[1] = new QStandardItemModel();
    model_mood[2] = new QStandardItemModel();
    model_mood[3] = new QStandardItemModel();

    ui->lv_import->setModel(model);
    ui->lv_nn->setModel(model_mood[0]);
    ui->lv_np->setModel(model_mood[1]);
    ui->lv_pn->setModel(model_mood[2]);
    ui->lv_pp->setModel(model_mood[3]);

    playback = new PlaybackThread("PLAY");
    predict = new PlaybackThread("PREDICT");

    //music_icon.addPixmap(QCoreApplication::applicationDirPath()+tr("/resources/minitunes.png"));
    music_icon.addPixmap(tr(":/icon"));
    connect(predict,SIGNAL(predict_result(int)),this,SLOT(predict_slot(int)));
    connect(this->ui->lv_import,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(play_double_clicked(QModelIndex)));
    connect(this->ui->lv_nn,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(play_double_clicked(QModelIndex)));
    connect(this->ui->lv_np,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(play_double_clicked(QModelIndex)));
    connect(this->ui->lv_pn,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(play_double_clicked(QModelIndex)));
    connect(this->ui->lv_pp,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(play_double_clicked(QModelIndex)));
}

MainWindow::~MainWindow()
{

    predict->stop();
    playback->stop();
    delete currentPredict;
    delete playback;
    delete predict;
    delete model_mood[0];
    delete model_mood[1];
    delete model_mood[2];
    delete model_mood[3];
    delete model;
    delete ui;
}

void MainWindow::on_pb_import_clicked()
{   
    QStringList fileNames = QFileDialog::getOpenFileNames(this,tr("Import Music File"), LAST_IMPORT_DIR, tr(".mf/Music Files (*.mp3 *.au *.wav *.mf )"));
    if(fileNames.length()>0){
        QString fileName = fileNames.at(0);
        LAST_IMPORT_DIR = fileName.mid(0,fileName.lastIndexOf('/'));
        //msg(LAST_IMPORT_DIR);
        QStandardItem *si;
        foreach(QString fn,fileNames){
            if(fileName.endsWith(".mf"))
            {
                Collection tmpCollection;
                tmpCollection.read(fn.toUtf8().data());
                QString tmpEntry;
                for(int i = 0;i< tmpCollection.size();i++)
                {
                    tmpEntry = QString::fromUtf8(tmpCollection.entry(i).c_str());
                    si = new QStandardItem();
                    si->setBackground(QBrush(QColor(0,0,0,0)));
                    si->setIcon(music_icon);
                    si->setData(tmpEntry);
                    tmpEntry = tmpEntry.mid(tmpEntry.lastIndexOf('/')+1,tmpEntry.length()-1);// for *nix only
                    si->setText(tmpEntry);
                    if(model->findItems(tmpEntry).length()<=0)
                        model->appendRow(si);
                }
            }else{
                fileName = fn.mid(fn.lastIndexOf('/')+1,fn.length()-1);// for *nix only
                si = new QStandardItem();
                si->setBackground(QBrush(QColor(0,0,0,0)));
                si->setIcon(music_icon);
                si->setText(fileName);
                si->setData(fn);
                if(model->findItems(fileName).length()<=0)
                    model->appendRow(si);
            }

        }
    }
}

void MainWindow::on_pb_predict_clicked()
{
    if(ui->lv_import->currentIndex().isValid())
    {
        //predict
        QStandardItem *si = model->itemFromIndex(ui->lv_import->currentIndex());
        QString sfName = si->data().toString();
        ui->label->setText("PREDICTING:" + sfName);

        QString this_path = QCoreApplication::applicationDirPath();
        string song = sfName.toUtf8().data();
        string song_weka = string(this_path.toUtf8().data()) + "/resources/single_.arff";
        string train_weka = string(this_path.toUtf8().data()) + "/resources/mc_train.arff";
        if(!predict->isRunning())
        {
            predict->song = song;
            predict->song_weka = song_weka;
            predict->train_weka = train_weka;
            currentPredict = si->clone();
            predict->start();
        }
    }
    else
    {
        QMessageBox::critical(this,"Wrong","Please import at least *one* song!");
    }

}



void MainWindow::on_pb_playback_clicked()
{
    if(playback->isRunning())
    {
        playback->stop();
        playback->msleep(300);
    }else{
        //set audio source
        playback->audioFile = this->currentSong;
        playback->start();
    }
}


void MainWindow::play_double_clicked(const QModelIndex &index)
{
    if(playback->isRunning())
    {
        playback->stop();
    }
    playback->msleep(300);
    //set audio source
    playback->audioFile = this->currentSong;
    playback->start();
}

void MainWindow::on_lv_import_clicked(const QModelIndex &index)
{
    this->currentSong = model->itemFromIndex(index)->data().toString();
    this->ui->label->setText(this->currentSong);
}

void MainWindow::on_lv_np_clicked(const QModelIndex &index)
{
    this->currentSong = model_mood[NP]->itemFromIndex(index)->data().toString();
    this->ui->label->setText(this->currentSong);
}

void MainWindow::on_lv_pp_clicked(const QModelIndex &index)
{
    this->currentSong = model_mood[PP]->itemFromIndex(index)->data().toString();
    this->ui->label->setText(this->currentSong);
}

void MainWindow::on_lv_nn_clicked(const QModelIndex &index)
{
    this->currentSong = model_mood[NN]->itemFromIndex(index)->data().toString();
    this->ui->label->setText(this->currentSong);
}

void MainWindow::on_lv_pn_clicked(const QModelIndex &index)
{
    this->currentSong = model_mood[PN]->itemFromIndex(index)->data().toString();
    this->ui->label->setText(this->currentSong);
}

void MainWindow::predict_slot(int mood)
{
    model_mood[mood]->appendRow(currentPredict);
}

void MainWindow::on_pb_output_clicked()
{
    Collection collection[5];
    for(int i = 0 ;i < model->rowCount();i++ ){
        collection[4].add(model->item(i)->data().toString().toUtf8().data(),"unknown");
    }
    collection[4].write("output_all.mf");

    string MOOD_DESC[4] = {"nn","np","pn","pp"};
    for(int i = 0 ;i < 4;i++ ){
        for(int j = 0 ;j < model_mood[i]->rowCount();j++ ){
            collection[i].add(model_mood[i]->item(j)->data().toString().toUtf8().data(),MOOD_DESC[i]);
        }
        collection[i].write("output_"+MOOD_DESC[i]+".mf");
    }

}
