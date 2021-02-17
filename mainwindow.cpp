#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QFontDialog>
#include <bits/stdc++.h>
#include <QStack>
#include <QString>
#include <QtDebug>
#include <QQueue>

///HUFFMAN TREE NODE
class node
{
public:
    int num;
    QChar ch;
    node *left,*right;
    node(int a=0,QChar b='*')
    {
        num=a;
        ch=b;
        left=NULL;
        right=NULL;
    }
};

///FOR PRIORITY QUEUE
class QPriorityQueue{
public:
    long long s;
    QVector<node*> arr;
    QPriorityQueue()
    {
        s=0;
    }
    void push(node* n){
        int stem = s;
        node* temp;
        arr.push_back(n);
        while( stem>0 ){
            if(arr[s/2]->num<arr[s]->num){
                temp = arr[s];
                arr[s] = arr[s/2];
                arr[s/2] = temp;
            }
            else
                break;
        }
        s++;
    }
    node* top()
    {
        return arr[0];
    }
    bool empty()
    {
        return arr.empty();
    }
    int size()
    {
        return s;
    }
    void min_heapify(int pos){
        int left = 2*pos + 1;
        int right = 2*pos + 2;
        node* temp;
        int min = pos;
        if(left < s && (arr[left]->num<arr[min]->num))
            min = left;
        if(right < s && (arr[right]->num<arr[min]->num))
            min = right;
        if(pos != min){
            temp = arr[min];
            arr[min] = arr[pos];
            arr[pos] = temp;
            min_heapify(min);
        }
    }
   void pop(){

        if(s>0){
            arr[0] = arr[s-1];
            s--;
            arr.pop_back();
            min_heapify(0);
        }
    }
};

///FOR SAVING DATA IN TEMPORARY FILE
class file_obj
{
public:
    QString name;
    QString decode;
    int ori;
    file_obj(QString nam="",QString decod="",int orig=0){
        name = nam;
        decode = decod;
        ori = orig;
    }
};
///Operator overloading
QDataStream &operator<<(QDataStream &out, const file_obj &file);
QDataStream &operator>>(QDataStream &in, file_obj &file);
//----OUT OVERLOAD-----
QDataStream &operator<<(QDataStream &out, const file_obj &file)
{
    out << file.name << file.decode
        << quint32(file.ori);
    return out;
}
//-----IN OVERLOAD--------
QDataStream &operator>>(QDataStream &in, file_obj &file)
{
    QString name="";
    QString decode="";
    quint32 ori=0;
    in >> name >> decode >> ori;
    file = file_obj(name,decode,ori);
    return in;
}
///GLOBAL VARIABLES
QString temporary="temporary.dat";
int siz;
QString original_text="";

///FOR COMPRESSION/ENCRYPTION
void getting_decoded_value(QMap<QChar,QVector<bool> > &decode,node *root,QVector<bool> temp)
{
    if(root==NULL)
        return;
    if(root->ch!='*')
    {
        decode.insert(root->ch,temp);
        return;
    }
    temp.push_back(true);
    getting_decoded_value(decode,root->left,temp);
    temp.pop_back();
    temp.push_back(false);
    getting_decoded_value(decode,root->right,temp);
    temp.pop_back();
}

///CREATING HUFFMAN TREE
node* huffman_code(QString str,QMap<QChar,QVector<bool> > &decode)
{
    QMap<QChar,int> M;
    for(int i=0;i<str.size();i++)
    {
        QChar si = str[i];
        QMap<QChar,int>::iterator it=M.find(si);
        if(it==M.end()||M.empty())
            M.insert(si,1);
        else
            it.value()++;
    }

    QPriorityQueue Q;
    for(QMap<QChar,int>::iterator it=M.begin();it!=M.end();it++)
    {
        node *newnode=new node(it.value(),it.key());
        Q.push(newnode);
    }
    int check = 0; // To check if only single character is to be stored in a file
    node *temp;
    while(Q.size()!=1)
    {
        check = 1;
        node *t1=Q.top();
        Q.pop();
        node *t2=Q.top();
        Q.pop();
        temp=new node(t1->num+t2->num);
        temp->left=t1;
        temp->right=t2;
        Q.push(temp);
    }
    if(check==0){
        node *t1 = Q.top();
        node *t2 = new node(0); //As the tree in huffman coding can not have just a single child
        temp=new node(t1->num+t2->num);
        temp->left=t1;
    }
    node *root=temp;

    ///DECODE
    QVector<bool> t;
    getting_decoded_value(decode,root,t);
    return root;
}

///GET STRING CORRESPONDING TO HUFFMAN TREE
void get_string(node *root,QString &str)
{
    if(root==NULL)
    {
        str+='~';
        return;
    }
    str+=root->ch;
    get_string(root->left,str);
    get_string(root->right,str);
}

///COMPRESS SOURCE FILE
void compress_file(QString &str,QString file_path)
{
    QMap<QChar,QVector<bool> > decode;

    node* root=huffman_code(str,decode);
    QQueue<bool> final_array;
    QString answer;
    for(int i=0;i<str.size();i++)
    {
        QMap<QChar,QVector<bool>>::iterator it=decode.find(str[i]);
        QVector<bool> temp=it.value();
        for(int i=0;i<temp.size();i++)
            final_array.enqueue(temp[i]);
        while(final_array.size()>=6)
        {
            int num=0,mult=32;
            for(int i=0;i<6;i++,mult/=2)
            {
                int temp=0;
                if(final_array.head()==true)
                    temp++;
                final_array.dequeue();
                num+=(temp*mult);
            }
            char charnum=num+64;
            answer+=charnum;
        }
    }

    int x=final_array.size();
    int num=0,mult=1;
    QStack<bool> S;
    while(!final_array.empty())
    {
        S.push(final_array.head());
        final_array.dequeue();
    }
    while(!S.empty())
    {
        int temp=S.top();
        S.pop();
        num+=(mult*temp);
        mult*=2;
    }
    char charnum=num+64;
    answer+=charnum;

    str=answer;
    file_obj F;
    get_string(root,F.decode);
    F.ori=x;
    F.name=file_path;

    QFile fin(temporary);
    QFile fout2("textfile.dat");
    if(fin.open(QIODevice::ReadOnly))
    {
        fout2.open(QIODevice::WriteOnly);
        QDataStream in(&fin);
        QDataStream oute(&fout2);
        while(!fin.atEnd())
        {
            file_obj t;
            in>>t;
            if(t.name==F.name)
                continue;
            oute<<t;
        }
        fin.close();
        fout2.flush();
        fout2.close();
        remove(temporary.toLatin1());
        rename("textfile.dat",temporary.toLatin1());
    }
    QFile fout(temporary);
    fout.open(QIODevice::WriteOnly | QIODevice::Append);
    QDataStream out(&fout);
    out<<(file_obj)F;
    fout.flush();
    fout.close();
}

///CREATE HUFFMAN TREE FROM STRING SAVED IN TEMPORARY FILE
node* create_tree(QString arr)
{
    if(siz==arr.size() || arr[siz]=='~')
    {
        siz++;
        return NULL;
    }
    node* root=new node(0,arr[siz]);
    siz++;
    root->left=create_tree(arr);
    root->right=create_tree(arr);
    return root;
}

///DECODING HUFFMAN FILE
QString decoder(QString str,int ori_size,QMap<QChar,QVector<bool> > decode)
{
    QMap<QVector<bool>,QChar> M;
    for(QMap<QChar,QVector<bool>>::iterator it=decode.begin();it!=decode.end();it++)
        M.insert(it.value(),it.key());
    QString answer;
    QQueue<bool> arr;
    for(int i=0;i<str.size();i++)
    {
        int temp;
        temp=str[i].toLatin1()-64;

        QStack<bool> S;
        int last=6;
        if(str.size()-1==i)
            last=ori_size;
        for(int j=0;j<last;j++)
        {
            if(temp%2==0)
                S.push(false);
            else
                S.push(true);
            temp/=2;
        }
        while(!S.empty())
        {
            arr.enqueue(S.top());
            S.pop();
        }
        QQueue<bool> temp_queue=arr;
        QVector<bool> temp_vector;
        while(!arr.empty())
        {
            temp_vector.push_back(arr.head());
            arr.dequeue();
            QMap<QVector<bool>,QChar>::iterator it=M.find(temp_vector);
            if(it==M.end())
                continue;
            else
            {
                answer+=it.value();
                temp_queue=arr;
                temp_vector.clear();
            }

        }
        arr=temp_queue;
    }
    return answer;
}

///UNCOMPRESS HUFFMAN FILE
QString uncompress_file(QString destination,QString str)
{
    file_obj F;
    QMap<QChar,QVector<bool> > decode;

    int ori_size=0;
    QString xyz;
    QFile fin(temporary);
    fin.open(QIODevice::ReadOnly );
    QDataStream in(&fin);
    while(!fin.atEnd())
    {
        in>>F;
        if(F.name==destination)
        {
            xyz = F.decode;
            ori_size=F.ori;
            break;
        }
    }
    fin.close();
    siz=0;
    node *root=create_tree(xyz);
    QVector<bool> a;
    getting_decoded_value(decode,root,a);


    return decoder(str,ori_size,decode);
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->textEdit);
}

void MainWindow::on_actionNew_triggered()
{
    file_path ="";  // set to empty string so whenever somebody clicks new button file path will be reseted
    ui->textEdit->setText("");  // to clear the text edit
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_name=QFileDialog::getOpenFileName(this,"Open the file");
    QFile file(file_name);
    file_path=file_name;
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this,"..","file not open");
        return ;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    //decompress the file
    text = uncompress_file(file_path,text);
    original_text=text;
    ui->textEdit->setText(text);  // put it in text editor
    file.close();

}

void MainWindow::on_actionSave_triggered()
{

    QFile file(file_path);

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,"..","file not open");
        return ;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
}

void MainWindow::on_actionSave_As_triggered()
{
    QString file_name=QFileDialog::getSaveFileName(this,"Open the file");
    QFile file(file_name);
    file_path=file_name;
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,"..","file not open");
        return ;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    original_text=text;
    //compress the text
    compress_file(text,file_path);
    out << text;  // put text in file
    file.flush();
    file.close();  // close file
}

void MainWindow::on_actionCopy_triggered()
{
    ui->textEdit->copy();
}

void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    printer.setPrinterName("Desired printer name");
    QPrintDialog dialog(&printer,this);
    if(dialog.exec()==QDialog::Rejected) return ;
    ui->textEdit->print(&printer);
}

void MainWindow::on_actionAbout_Notepad_triggered()
{
    QString about_text;
    about_text = "Advanced Notepad\n";
    about_text += "Author : Surajit Mondal & Kiran Maharana\n";
    about_text += "Date : 17/10/2020\n";

    QMessageBox::about(this,"About Notepad",about_text);
}

void MainWindow::on_actionFont_triggered()
{

    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);
    if (fontSelected)
        ui->textEdit->setFont(font);
}
