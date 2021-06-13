//KLIENT

#pragma once
#include <sstream>
#include <WS2tcpip.h>


#pragma comment (lib, "ws2_32.lib")

namespace CppCLR_WinformsProjekt {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace std;

	public ref class Form1 : public System::Windows::Forms::Form
	{	
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  button9;
	private: System::Windows::Forms::Button^  button8;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Label^  label2;
	
	public:
		bool isGame = true;
		int buttonIndex = 0;
		String^ title = "Gracz O";
		String^ mark = "O";
		SOCKET sock;

// WCZYTYWANIE PLANSZY I TWORZENIE ARCHITEKTURY KLIENT SERWER

		Form1(void) // Wczytywanie planszy i tworzenie polaczenia 
		{
			
			InitializeComponent();

			string ipAddress = "127.0.0.1";			// Adres IP serwera
			int port = 54000;						// Numer portu serwera

			WSAData data;
			WORD ver = MAKEWORD(2, 2);
			int wsResult = WSAStartup(ver, &data);
			if (wsResult != 0)
			{
				return;
			}

			// Tworzenie socketu
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock == INVALID_SOCKET)
			{
				WSACleanup();
				return;
			}

			// Uzupelnienie struktury odpowiedzi
			sockaddr_in hint;
			hint.sin_family = AF_INET;
			hint.sin_port = htons(port);
			inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

			// Polaczenie z serwerem
			int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
			if (connResult == SOCKET_ERROR)
			{
				if (MessageBox::Show("Blad polaczenia z serwerem. Ponawianie proby", "", MessageBoxButtons::OKCancel,
					MessageBoxIcon::Error) == System::Windows::Forms::DialogResult::OK)
				{
					int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
				}
				else
				{
					EndGame();
					return;
				}
			}

			MessageBox::Show("Polaczono z portem! Jestes graczem O");

			label1->Text = title;
			label2->Text = "Twoj ruch";
		}

		void ClientCodeSend() //wysylanie wiadomosci do serwera
		{

			char recvBuf[4096];
			
			//konwersja na typ do wyslania
			stringstream strs;
			strs << buttonIndex; //konwersja int->stringstream
			string sendBuf = strs.str(); //konwersja stringstream->string

			int sendResult = send(sock, sendBuf.c_str(), sendBuf.size() + 1, 0);

			if (sendResult != SOCKET_ERROR) //Jesli wiadomosc zostala wyslana poprawnie 
			{
				label2->Text = "Ruch przeciwnika";
				
				this->Refresh();

				if (Winning(mark))
				{
					label2->Text = "WYGRALES";
					if (MessageBox::Show("Wygrana! Czy chcesz zagrac ponownie?", "", MessageBoxButtons::YesNo,
						MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
					{
						EndGame();
					}
					else
					{
						Application::Restart();
					}
				}
				else if (Remis())
				{
					label2->Text = "REMIS";
					if (MessageBox::Show("Remis! Czy chcesz zagrac ponownie?", "", MessageBoxButtons::YesNo,
						MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
					{
						EndGame();
					}
					else
					{
						Application::Restart();
					}
				}

				ClientCodeRecv();
			}
			else
			{
				MessageBox::Show("Wysylanie nie powiodlo sie");
			}

		}

		void ClientCodeRecv() //Odbieranie wiadomosci od serwera
		{
			char recvBuf[4096];

			while (true)
			{
				WaitForResponse(); //blokowanie przycoskow -> czekanie na odpowiedz serwera 
				ZeroMemory(recvBuf, 4096);
				// Oczekiwanie na wiadomosc serwera

				int bytesReceived = recv(sock, recvBuf, 4096, 0);

				if (bytesReceived == SOCKET_ERROR)
				{
					MessageBox::Show("Blad polaczenia. Koniec pracy programu.");
					EndGame();
					break;
				}

				if (bytesReceived == 0)
				{
					MessageBox::Show("Utracono polaczenie z graczem. Koniec pracy programu.");
					EndGame();
					break;

				}

				// Konwersja typu do odczytu
				string response = string(recvBuf, 0, bytesReceived); //konwersja char->string
				String^ message = gcnew String(response.c_str()); //konwersja string->String^

				if (message != "")
				{
					AddOpponentMark(message);
					if (Winning("X"))
					{
						label2->Text = "PRZEGRALES";
						if (MessageBox::Show("Przegrana! Czy chcesz zagrac ponownie?", "", MessageBoxButtons::YesNo,
							MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
						{
							EndGame();
						}
						else
						{
							Application::Restart();
						}
					}
					else if (Remis())
					{
						label2->Text = "REMIS";
						if (MessageBox::Show("Remis! Czy chcesz zagrac ponownie?", "", MessageBoxButtons::YesNo,
							MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
						{
							EndGame();
						}
						else
						{
							Application::Restart();
						}
					}

					label2->Text = "Twoj ruch";

					this->Refresh();
				}
				break;
			}

			TurnOnButtons(); //Odblokowanie przyciskow
		}

		void WaitForResponse() //blokowanie przyciskow
		{
			button1->Enabled = false;
			button2->Enabled = false;
			button3->Enabled = false;
			button4->Enabled = false;
			button5->Enabled = false;
			button6->Enabled = false;
			button7->Enabled = false;
			button8->Enabled = false;
			button9->Enabled = false;

		}

		void TurnOnButtons() //odblokowywanie przyciskow
		{
			button1->Enabled = true;
			button2->Enabled = true;
			button3->Enabled = true;
			button4->Enabled = true;
			button5->Enabled = true;
			button6->Enabled = true;
			button7->Enabled = true;
			button8->Enabled = true;
			button9->Enabled = true;
		}

// LOGIKA GRY

		bool Occupied(Button^ field) //sprawdzanie czy pole jest zajete (jesli tak, zwraca true)
		{
			if (field->Text != "")
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		bool MarkAdded(Button^ field) //dodawanie znaku -> jesli udalo sie wstawic znak (pole bylo wolne) zwraca true
		{
			if (Occupied(field))
			{
				MessageBox::Show("To pole jest juz zajete!");
				return false;
			}

			field->Text = mark;

			buttonIndex = field->TabIndex;
			this->Refresh();

			return true;

		}

		void AddOpponentMark(String^ index) //dodawanie znaku przeciwnika (nic nie sprawdza, serwer juz sprawdzil wolne pola)
		{
			if (index == "1")
			{
				button1->Text = "X";
			}
			else if (index == "2")
			{
				button2->Text = "X";
			}
			else if (index == "3")
			{
				button3->Text = "X";
			}
			else if (index == "4")
			{
				button4->Text = "X";
			}
			else if (index == "5")
			{
				button5->Text = "X";
			}
			else if (index == "6")
			{
				button6->Text = "X";
			}
			else if (index == "7")
			{
				button7->Text = "X";
			}
			else if (index == "8")
			{
				button8->Text = "X";
			}
			else if (index == "9")
			{
				button9->Text = "X";
			}
			else
			{
				MessageBox::Show("Nie udalo sie przypisac znaku");
			}
		}

		bool Winning(String^ playerMark) //Sprawdzanie wygranej i zmiana kolor pol
		{
			//poziomo
			if (button1->Text == playerMark && button2->Text == playerMark && button3->Text == playerMark)
			{
				button1->BackColor = System::Drawing::Color::NavajoWhite;
				button2->BackColor = System::Drawing::Color::NavajoWhite;
				button3->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
			if (button4->Text == playerMark && button5->Text == playerMark && button6->Text == playerMark)
			{
				button4->BackColor = System::Drawing::Color::NavajoWhite;
				button5->BackColor = System::Drawing::Color::NavajoWhite;
				button6->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
			if (button7->Text == playerMark && button8->Text == playerMark && button9->Text == playerMark)
			{
				button7->BackColor = System::Drawing::Color::NavajoWhite;
				button8->BackColor = System::Drawing::Color::NavajoWhite;
				button9->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}

			//pionowo
			if (button1->Text == playerMark && button4->Text == playerMark && button7->Text == playerMark)
			{
				button1->BackColor = System::Drawing::Color::NavajoWhite;
				button4->BackColor = System::Drawing::Color::NavajoWhite;
				button7->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
			if (button2->Text == playerMark && button5->Text == playerMark && button8->Text == playerMark)
			{
				button2->BackColor = System::Drawing::Color::NavajoWhite;
				button5->BackColor = System::Drawing::Color::NavajoWhite;
				button8->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
			if (button3->Text == playerMark && button6->Text == playerMark && button9->Text == playerMark)
			{
				button3->BackColor = System::Drawing::Color::NavajoWhite;
				button6->BackColor = System::Drawing::Color::NavajoWhite;
				button9->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}

			//ukos
			if (button1->Text == playerMark && button5->Text == playerMark && button9->Text == playerMark)
			{
				button1->BackColor = System::Drawing::Color::NavajoWhite;
				button5->BackColor = System::Drawing::Color::NavajoWhite;
				button9->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
			if (button3->Text == playerMark && button5->Text == playerMark && button7->Text == playerMark)
			{
				button3->BackColor = System::Drawing::Color::NavajoWhite;
				button5->BackColor = System::Drawing::Color::NavajoWhite;
				button7->BackColor = System::Drawing::Color::NavajoWhite;
				return true;
			}
		}

		bool Remis() //gdy nie zostanie juz zadne puste pole
		{
			if (button1->Text != "" && button2->Text != "" && button3->Text != "" && button4->Text != "" && button5->Text != "" && button6->Text != "" && button7->Text != "" && button8->Text != "" && button9->Text != "")
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		void EndGame() //zamykanie socketow i aplikacji
		{
			Application::Exit();
			closesocket(sock);
			WSACleanup();
		}


// TWORZENIE PLANSZY

	protected:
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code

		void InitializeComponent(void)
		{
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->label1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 16.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->label1->Location = System::Drawing::Point(86, 33);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(210, 48);
			this->label1->TabIndex = 0;
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// button9
			// 
			this->button9->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button9->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button9->Location = System::Drawing::Point(233, 239);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(72, 65);
			this->button9->TabIndex = 9;
			this->button9->UseVisualStyleBackColor = true;
			this->button9->Click += gcnew System::EventHandler(this, &Form1::button9_Click);
			// 
			// button8
			// 
			this->button8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button8->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button8->Location = System::Drawing::Point(155, 239);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(72, 65);
			this->button8->TabIndex = 8;
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &Form1::button8_Click);
			// 
			// button7
			// 
			this->button7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button7->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button7->Location = System::Drawing::Point(77, 239);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(72, 65);
			this->button7->TabIndex = 7;
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &Form1::button7_Click);
			// 
			// button6
			// 
			this->button6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button6->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button6->Location = System::Drawing::Point(233, 168);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(72, 65);
			this->button6->TabIndex = 6;
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &Form1::button6_Click);
			// 
			// button5
			// 
			this->button5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button5->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button5->Location = System::Drawing::Point(155, 168);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(72, 65);
			this->button5->TabIndex = 5;
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
			// 
			// button4
			// 
			this->button4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button4->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button4->Location = System::Drawing::Point(77, 168);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(72, 65);
			this->button4->TabIndex = 4;
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click);
			// 
			// button3
			// 
			this->button3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button3->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button3->Location = System::Drawing::Point(233, 97);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(72, 65);
			this->button3->TabIndex = 3;
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// button2
			// 
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button2->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button2->Location = System::Drawing::Point(155, 97);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(72, 65);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button1
			// 
			this->button1->BackColor = System::Drawing::SystemColors::ControlLight;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button1->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button1->Location = System::Drawing::Point(77, 97);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(72, 65);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = false;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// label2
			// 
			this->label2->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->label2->Location = System::Drawing::Point(74, 328);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(231, 23);
			this->label2->TabIndex = 20;
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(379, 376);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->button9);
			this->Controls->Add(this->button8);
			this->Controls->Add(this->button7);
			this->Controls->Add(this->button6);
			this->Controls->Add(this->button5);
			this->Controls->Add(this->button4);
			this->Controls->Add(this->button3);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->label1);
			this->Margin = System::Windows::Forms::Padding(4);
			this->Name = L"Form1";
			this->Text = L"Kolko-Krzyzyk";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResumeLayout(false);

		}
#pragma endregion

// FUNKCJE PRZYCISKOW

	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
		
	}
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		if (MarkAdded(button1))
		{
			ClientCodeSend();
		}
	}
private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button2))
	{
		ClientCodeSend();
	}
}
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button3))
	{
		ClientCodeSend();
	}
}
private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button4))
	{
		ClientCodeSend();
	}
}
private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button5))
	{
		ClientCodeSend();
	}
}
private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button6))
	{
		ClientCodeSend();
	}
}
private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button7))
	{
		ClientCodeSend();
	}
}
private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button8))
	{
		ClientCodeSend();
	}
}
private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {
	if (MarkAdded(button9))
	{
		ClientCodeSend();
	}
}
};
}
