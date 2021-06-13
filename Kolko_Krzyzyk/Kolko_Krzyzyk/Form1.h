//SERWER

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
	public:

		
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::Button^  button8;
	private: System::Windows::Forms::Button^  button9;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Button^  button10;
	private: System::Windows::Forms::Label^  label2;
	
	public:
		bool isGame = true;
		int buttonIndex = 0;
		String^ title = "Gracz X";
		String^ mark = "X";
		SOCKET clientSocket;
		
//WCZYTANIE PLANSZY I ARCHITEKTURA KLIENT-SERWER
		
		Form1(void) 
		{
			InitializeComponent(); //Wczytywanie planszy
			
		}
		
		void ServerCode() //Tworzenie polaczenia
		{
			WSADATA wsData;
			WORD ver = MAKEWORD(2, 2);

			int wsOk = WSAStartup(ver, &wsData);
			if (wsOk != 0)
			{
				MessageBox::Show("Can't Initialize winsock");
				return;
			}

			// Tworzenie socketu
			SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
			if (listening == INVALID_SOCKET)
			{
				MessageBox::Show("Can't create a socket!");
				return;
			}

			// wi¹zanie (bind) portu i adresu ip z socketem 
			sockaddr_in hint;
			hint.sin_family = AF_INET;
			hint.sin_port = htons(54000);
			hint.sin_addr.S_un.S_addr = INADDR_ANY; 

			bind(listening, (sockaddr*)&hint, sizeof(hint));

			// Przypisanie roli sluchania 
			listen(listening, SOMAXCONN);

			// Czekanie na polaczenie
			sockaddr_in client;
			int clientSize = sizeof(client);


			clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
			char host[NI_MAXHOST];		// Nazwa klienta
			char service[NI_MAXSERV];	// Port z ktorym laczy sie klient

			ZeroMemory(host, NI_MAXHOST); 
			ZeroMemory(service, NI_MAXSERV);

			if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
			{

				MessageBox::Show("Polaczona z portem. Jestes graczem X");

				
			}
			else
			{
				inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);

				MessageBox::Show("Polaczono z portem" + ntohs(client.sin_port) + "Jestes graczem X");

			}

			label1->Text = title;
			this->Refresh();

			closesocket(listening);

		}

		void ServerCodeRecv() //Odbieranie wiadomosci od klienta
		{
			label2->Text = "Ruch przeciwnika";
			this->Refresh();

			char recvBuf[4096];

			while (true)
			{
				WaitForResponse(); //blokowanie przyciskow
				
				ZeroMemory(recvBuf, 4096);
				// Oczekiwanie na odpowiedz klienta
				int bytesReceived = recv(clientSocket, recvBuf, 4096, 0);

				if (bytesReceived == SOCKET_ERROR)
				{
					MessageBox::Show("Blad polaczenia. Konczenie pracy programu.");
					EndGame();
					break;
				}

				if (bytesReceived == 0)
				{
					MessageBox::Show("Utracono polaczenie z graczem. Konczenie pracy programu.");
					EndGame();
					break;
				}

				//konwersja typu do odczytu
				string response = string(recvBuf, 0, bytesReceived); //konwersja char->string
				String^ message = gcnew String(response.c_str()); //konwersja string->String^

				if (message != "")
				{
					AddOpponentMark(message);

					if (Winning("O"))
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
				}
				break;
			}

			TurnOnButtons(); //Odblokowanie przyciskow
			label2->Text = "Twoj ruch";
			this->Refresh();
		}

		void ServerCodeSend() //wysylanie wiadomosci do klienta
		{
			char recvBuf[4096];
			
			//konwersja typu do wyslania
			stringstream strs;
			strs << buttonIndex; //konwersja int->stringstream
			string sendBuf = strs.str(); //konwersja stringstream->string

			int sendResult = send(clientSocket, sendBuf.c_str(), sendBuf.size() + 1, 0);

			if (sendResult != SOCKET_ERROR) //jesli wiadomosc zostala wyslana poprawnie
			{
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
				label2->Text = "Ruch przeciwnika";
				this->Refresh();

				ServerCodeRecv();
				label2->Text = "Twoj ruch";
				this->Refresh();

			}
			else
			{
				MessageBox::Show("Wysylanie nie powiodlo sie");
			}


		}

		void WaitForResponse() //Blokowanie przycoskow - czekanie na ruch przeciwnika
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

		void TurnOnButtons() //Odblokowywanie przyciskow
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

		bool Occupied(Button^ field) //Sprawdzanie czy dane pole jest zajete (jesli tak, zwraca true)
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
	

		bool AddMark(Button^ field) //dodawanie znaku do planszy -> jesli znak zostal poprawnie dodany (pole nie bylo zajete), zwraca true
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

		void AddOpponentMark(String^ index) //dodawanie znaku przeciwnika do planszy (nic nie sprawdza, dostepnosc pol sprawdzil juz przeciwnik)
		{
			if (index == "1")
			{
				button1->Text = "O";
			}
			else if (index == "2")
			{
				button2->Text = "O";
			}
			else if (index == "3")
			{
				button3->Text = "O";
			}
			else if (index == "4")
			{
				button4->Text = "O";
			}
			else if (index == "5")
			{
				button5->Text = "O";
			}
			else if (index == "6")
			{
				button6->Text = "O";
			}
			else if (index == "7")
			{
				button7->Text = "O";
			}
			else if (index == "8")
			{
				button8->Text = "O";
			}
			else if (index == "9")
			{
				button9->Text = "O";
			}
			else
			{
				MessageBox::Show("Nie udalo sie przypisac znaku");
			}
		}

		bool Winning(String^ playerMark) //Sprawdzanie wygranej i zmiana koloru pol wygrywajacych
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

		void EndGame() //koniec gry -> zamykanie socketow i aplikacji
		{
			closesocket(clientSocket);

			// Cleanup winsock
			WSACleanup();

			system("pause");

			Application::Exit();
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
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button10 = (gcnew System::Windows::Forms::Button());
			this->panel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->label1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 16.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->label1->Location = System::Drawing::Point(81, 42);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(210, 48);
			this->label1->TabIndex = 0;
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->label1->Click += gcnew System::EventHandler(this, &Form1::label1_Click);
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button1->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button1->Location = System::Drawing::Point(72, 105);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(72, 65);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// button2
			// 
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button2->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button2->Location = System::Drawing::Point(150, 105);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(72, 65);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button3
			// 
			this->button3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button3->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button3->Location = System::Drawing::Point(228, 105);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(72, 65);
			this->button3->TabIndex = 3;
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// button4
			// 
			this->button4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button4->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button4->Location = System::Drawing::Point(72, 176);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(72, 65);
			this->button4->TabIndex = 4;
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click);
			// 
			// button5
			// 
			this->button5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button5->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button5->Location = System::Drawing::Point(150, 176);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(72, 65);
			this->button5->TabIndex = 5;
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
			// 
			// button6
			// 
			this->button6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button6->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button6->Location = System::Drawing::Point(228, 176);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(72, 65);
			this->button6->TabIndex = 6;
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &Form1::button6_Click);
			// 
			// button7
			// 
			this->button7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button7->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button7->Location = System::Drawing::Point(72, 247);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(72, 65);
			this->button7->TabIndex = 7;
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &Form1::button7_Click);
			// 
			// button8
			// 
			this->button8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button8->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button8->Location = System::Drawing::Point(150, 247);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(72, 65);
			this->button8->TabIndex = 8;
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &Form1::button8_Click);
			// 
			// button9
			// 
			this->button9->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 25.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(238)));
			this->button9->ForeColor = System::Drawing::SystemColors::WindowFrame;
			this->button9->Location = System::Drawing::Point(228, 247);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(72, 65);
			this->button9->TabIndex = 9;
			this->button9->UseVisualStyleBackColor = true;
			this->button9->Click += gcnew System::EventHandler(this, &Form1::button9_Click);
			// 
			// label2
			// 
			this->label2->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->label2->Location = System::Drawing::Point(69, 341);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(231, 23);
			this->label2->TabIndex = 11;
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// panel1
			// 
			this->panel1->Controls->Add(this->label3);
			this->panel1->Controls->Add(this->button10);
			this->panel1->Location = System::Drawing::Point(1, 2);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(376, 396);
			this->panel1->TabIndex = 12;
			// 
			// label3
			// 
			this->label3->Location = System::Drawing::Point(68, 148);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(251, 63);
			this->label3->TabIndex = 21;
			this->label3->Text = L"Po klikniêciu \"OK\" rozpocznie siê praca serwera";
			this->label3->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// button10
			// 
			this->button10->Location = System::Drawing::Point(111, 262);
			this->button10->Name = L"button10";
			this->button10->Size = System::Drawing::Size(147, 64);
			this->button10->TabIndex = 20;
			this->button10->Text = L"OK";
			this->button10->UseVisualStyleBackColor = true;
			this->button10->Click += gcnew System::EventHandler(this, &Form1::button10_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(378, 399);
			this->Controls->Add(this->panel1);
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
			this->Activated += gcnew System::EventHandler(this, &Form1::Form1_Activated);
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->panel1->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion

// FUNKCJE PRZYCISKOW

	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
		
		
	}
	private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) {
		
	}
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

	if (AddMark(button1))
	{
		ServerCodeSend();
	}
}

private: System::Void Form1_Activated(System::Object^  sender, System::EventArgs^  e) {
	
}
private: System::Void button10_Click(System::Object^  sender, System::EventArgs^  e) {

	panel1->Visible = false;
	this->Refresh();
	ServerCode();
	this->Refresh();
	ServerCodeRecv();
	this->Refresh();
}
private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button2))
	{
		ServerCodeSend();
	}
}
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button3))
	{
		ServerCodeSend();
	}
}
private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button4))
	{
		ServerCodeSend();
	}
}
private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button5))
	{
		ServerCodeSend();
	}
}
private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button6))
	{
		ServerCodeSend();
	}
}
private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button7))
	{
		ServerCodeSend();
	}
}
private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button8))
	{
		ServerCodeSend();
	}
}
private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {
	if (AddMark(button9))
	{
		ServerCodeSend();
	}
}
};
}
