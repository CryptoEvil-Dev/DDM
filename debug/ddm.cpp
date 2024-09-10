#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>


#include "termios.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

char c_getchar(){
	struct termios old, neww;
	char c;
	//printf("Press any key:");
	fflush(stdout);               
	tcgetattr(0,&old);            
	tcgetattr(0,&neww);
	neww.c_lflag&=~ICANON;         
	tcsetattr(0,TCSANOW,&neww);    
	read(0,&c,1);                 
	//printf("\nPressed - %c\n",c);
	tcsetattr(0,TCSANOW,&old);
	return c;
}

namespace ddm{

	class DDM{
	private:
		std::fstream fio;
		std::string config_file_path;
		std::vector<std::map<std::string, std::string>> shells;

		std::map<std::string, std::string> Parser(std::string config_path);

		std::string password;

		int selected = 0;
		int clicked;

		static const int width = 240;
		static const int height = 70;

		char* display = new char[width*height];
	public:
		void ClearScreen(){
			for(int i = 0; i < width * height; i++){
				display[i] = ' ';
			}
			system("clear");
		}
		void CastWindow(){
			for(int h = width * 1; h < width * height; h += width){
				display[h] = '|';
				display[h+width - 1] = '|';
			}
			for(int i = width * height - width; i < width * height; i++){
				display[i] = '_';
			}

			//system("clear");
			for(int i = 0; i <  width; i++){
				display[i] = '_';
			}
			display[width / 2 - 2] = ' ';
			display[width / 2 - 1] = 'D';
			display[width / 2]     = 'D';
			display[width / 2 + 1] = 'M';
			display[width / 2 + 2] = ' ';

			char help_txt[] = {"↑ - switch up; ↓ - switch down; enter - select; Backspace - load tty"};
			for(int i = 0; i < sizeof(help_txt); i++){
				display[(width * height) - sizeof(help_txt) + i] = help_txt[i];
			}


			char welcome_txt[] = {"DDM v0.1. Select your graphic shell!"};
			for(int i = 0; i < sizeof(welcome_txt)  - 1; i++){
				display[(int)((width * 3) + (width / 2 - sizeof(welcome_txt) / 2) + i)] = welcome_txt[i];
			}

			//std::cout << display << std::endl;
			for(int i = 0; i < width * height; i++){
				std::putchar(display[i]);
			}
		}
		void CastSelector(){
			char no_match[] = {"Graphic Shell's not found =("};
			if(this->shells.size() == 0){
				for(int i =0; i <  sizeof(no_match) - 1; i++){
					display[((width / 2) - (sizeof(no_match) / 2)) + (width * (height / 2)) + i] = no_match[i];
				}
			} else {
				if(this->clicked == 66){
					this->selected += 1;
				}
				if(this->clicked == 65){
					this->selected -= 1;
				}

				if(this->clicked == 10){
					//std::cout << this->shells[this->selected]["command"].c_str() << std::endl;
					system(this->shells[this->selected]["command"].c_str());
					exit(0);
				}


				if(this->selected > this->shells.size() - 1){
					this->selected = 0;
				}
				if(this->selected <= -1){
					this->selected = this->shells.size();
				}
				for(int sh = 0; sh < this->shells.size(); sh++){
					int pos = (width * sh) + (width * 10) + (width / 2);
					
					if(this->selected == sh){
						for(int i = 0; i < this->shells[sh]["name"].size(); i++){
							display[pos + i] = this->shells[sh]["name"][i];
						}
						display[pos - 2] = '>';
					}

					for(int i = 0; i < this->shells[sh]["name"].size(); i++){
						display[pos + i] = this->shells[sh]["name"][i];
					}


				}
				std::cout << this->selected << std::endl;
				std::cout << this->clicked << std::endl;
				system("echo $USER");
				std::cout << stdout << std::endl;
			}
		}

		void LoopShellSelector(){
			while(this->clicked != 127){
				
				//std::cout << this->clicked; // 66 вниз 65 вверх
				this->ClearScreen();
				this->CastSelector();
				this->CastWindow();
				this->clicked = (int)c_getchar();
			}
		}

		void Parser(){
			std::cout << this->config_file_path << std::endl;
			//std::cout << this->GetUsername() << std::endl;
			std::ifstream ifs = std::ifstream(this->config_file_path);
			
			if(!ifs.is_open()){
				std::cout << "\033[1;31mDDM panic! File not exist!\033[0m" << std::endl;
				exit(1);
			} else{
				std::string line;
				while(std::getline(ifs, line)){
					char* tmp = new char[line.size()];
					for(int i =0; i< line.size(); i++){
						tmp[i] = line[i];
					}
					// Поиск разделения name = command
					std::string name;
					std::string command;
					int split;
					for(int i = 0; i < line.size(); i++){
						if(tmp[i] == '='){
							split = i;
						}
					}
					char* tmp_name = new char[split -1];
					char* tmp_comm = new char[line.size() - split + 1];
					for(int i = 0; i < split - 1; i++){
						tmp_name[i] = tmp[i];
					}
					for(int i =  split + 1; i < line.size(); i++){
						tmp_comm[i - split - 1] = tmp[i];

					}
					name = tmp_name;
					command = tmp_comm;
					delete[] tmp_name;
					delete[] tmp_comm;
					this->AddShell(name, command);
					//std::cout << "Name: " << name << "  Command: " << command << std::endl;
					

				}
			}

			
			
			
			
		}

		std::string GetUsername(){
			char text[255];
    		FILE *name;
    		name = popen("whoami", "r");
    		fgets(text, sizeof(text), name);
    		//std::cout << "Name is : " << text;
    		pclose(name);
    		//std::cout << std::endl;
			
			for(int i = 0; i < sizeof(text); i++){
				if(text[i] == '\n'){
					text[i] = '\0';
					break;
				}
			}

			return (std::string)text;
		}

		DDM(){
			std::string name = this->GetUsername();
			this->config_file_path = "/home/" + name + "/.config/DDM/config.conf";
			this->Parser();
			this->LoopShellSelector();
		};
		DDM(std::string path_config){
			this->config_file_path = path_config;
			this->Parser();
			
			this->AddShell("Hyprland", "hyprland");
			this->AddShell("Wayland", "startplasma-wayland");
			this->AddShell("X11", "startplasma-x11");
			this->AddShell("Sway", "sway");

			this->LoopShellSelector();
			//this->Test();
			//std::cout << "\033[1;31m DDM panic! \033[0m\n";
		};


		~DDM(){
			delete[] display;
			//system("clear");
		}

	protected:
		void AddShell(std::string name, std::string command){
			std::map <std::string, std::string> new_shell;
			new_shell["name"] = name;
			new_shell["command"] = command;
			this->shells.push_back(new_shell);
		};
		void DelShell(std::string name);


	};


}


int main(int argc, char* argv[]){
	ddm::DDM d = ddm::DDM();
	//std::map<std::string, std::string> response = d.SelectLoop();
	//std::cout << response["code"] << std::endl;
	return 0;
}
