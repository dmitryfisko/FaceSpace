#pragma once

#include "opencv2/opencv.hpp"

namespace openCvTest {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing; 

    IplImage* frame = 0;
    CvCapture* capture = 0;
    int camBusy = 0;
    int camStatus = 0;

    /// <summary>
    /// Summary for Form1
    /// </summary>
    public ref class Form1 : public System::Windows::Forms::Form
    {
    public:
        Form1(void)
        {
            InitializeComponent();
            //
            //TODO: Add the constructor code here
            //
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~Form1()
        {
            if (components)
            {
                delete components;
            }
        }
    private: System::ComponentModel::BackgroundWorker^  backgroundWorker1;
    protected:
    private: System::Windows::Forms::PictureBox^  pictureBox1;
    private: System::Windows::Forms::Button^  button1;
    private: System::Windows::Forms::Button^  button2;

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
            this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->button2 = (gcnew System::Windows::Forms::Button());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
            this->SuspendLayout();
            // 
            // backgroundWorker1
            // 
            this->backgroundWorker1->WorkerReportsProgress = true;
            this->backgroundWorker1->WorkerSupportsCancellation = true;
            this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
            this->backgroundWorker1->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);
            // 
            // pictureBox1
            // 
            this->pictureBox1->Location = System::Drawing::Point(12, 12);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(640, 480);
            this->pictureBox1->TabIndex = 0;
            this->pictureBox1->TabStop = false;
            // 
            // button1
            // 
            this->button1->Location = System::Drawing::Point(12, 498);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(75, 23);
            this->button1->TabIndex = 1;
            this->button1->Text = L"CamStart";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
            // 
            // button2
            // 
            this->button2->Location = System::Drawing::Point(577, 498);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(75, 23);
            this->button2->TabIndex = 2;
            this->button2->Text = L"CamStop";
            this->button2->UseVisualStyleBackColor = true;
            this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(668, 544);
            this->Controls->Add(this->button2);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->pictureBox1);
            this->DoubleBuffered = true;
            this->Name = L"Form1";
            this->Text = L"Form1";
            this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
            this->ResumeLayout(false);

        }
#pragma endregion
    private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
    }
    private: System::Void backgroundWorker1_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e) {
        BackgroundWorker^ worker = dynamic_cast<BackgroundWorker^>(sender);
        capture = cvCreateCameraCapture(1);
        while (camStatus){
            if (camBusy) continue;
            frame = cvQueryFrame(capture);
            if (!frame) continue;
            worker->ReportProgress(1);
        }
        cvReleaseImage(&frame);
        cvReleaseCapture(&capture);
    }
    private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
        camStatus = 1;
        backgroundWorker1->RunWorkerAsync(10);
    }
    private: System::Void DrawCvImage(IplImage *CvImage, System::Windows::Forms::PictureBox^ pbx) {
        // typecast IplImage to Bitmap
        if ((pbx->Image == nullptr) || (pbx->Width != CvImage->width) || (pbx->Height != CvImage->height)){
            pbx->Width = CvImage->width;
            pbx->Height = CvImage->height;
            Bitmap^ bmpPicBox = gcnew Bitmap(pbx->Width, pbx->Height);
            pbx->Image = bmpPicBox;
        }

        Graphics^g = Graphics::FromImage(pbx->Image);

        Bitmap^ bmp = gcnew Bitmap(CvImage->width, CvImage->height, CvImage->widthStep,
                                   System::Drawing::Imaging::PixelFormat::Format24bppRgb, IntPtr(CvImage->imageData));

        g->DrawImage(bmp, 0, 0, CvImage->width, CvImage->height);
        pbx->Refresh();

        delete g;
    }
    private: System::Void backgroundWorker1_ProgressChanged(System::Object^  sender, System::ComponentModel::ProgressChangedEventArgs^  e) {
        if (!camBusy && camStatus){
            camBusy = 1;
            IplImage *destination = cvCreateImage(cvSize(640, 480), frame->depth, frame->nChannels);
            cvResize(frame, destination);
            DrawCvImage(destination, pictureBox1);
            cvReleaseImage(&destination);
            camBusy = 0;
        }
    }
    private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
        camStatus = 0;
    }
    };
}