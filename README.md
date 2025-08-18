# SuperClash Online
A fast-paced 2D multiplayer battle game built in C++ with ENet & Raylib.

## Features
- Create or join rooms and battle friends in real-time  
- Each player chooses a unique superpower  
- Smooth networking powered by ENet  
- Lightweight 2D engine-free design with Raylib  
- Cross-platform potential  

## Getting Started

### 1. Install Git
Download and install Git:  
https://github.com/git-for-windows/git/releases/download/v2.50.1.windows.1/Git-2.50.1-64-bit.exe  

### 2. Setup VCPKG & Dependencies
Open **CMD** and run:
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.bootstrap-vcpkg.bat
.vcpkg integrate install
.vcpkg install raylib:x64-windows
.vcpkg install enet:x64-windows
```

### 3. Configure in Visual Studio
- Go to Project → Properties  
- Select Configuration Properties → vcpkg  
- Set Use Vcpkg to "Yes"  

## Building the Game
1. Open the project in **Visual Studio Community**  
2. Build the solution (Ctrl + Shift + B)  
3. Run the executable from `bin/`  

## License
This project is licensed under the **MIT License** – see the LICENSE file for details.  

## Contributing
Pull requests are welcome! Feel free to open issues for bugs, feature requests, or ideas.  

## Credits
- C++ – Core language  
- Raylib – Rendering & input handling  
- ENet – Low-level networking library  
