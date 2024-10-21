#include <filesystem>
#include <iostream>

void CreateGameFolder(const std::string& folderPath) {
    std::filesystem::create_directory(folderPath);
}

void CopyImageToFolder(const std::string& imagePath, const std::string& folderPath) {
    std::filesystem::copy(imagePath, folderPath);
}

int main() {
    std::string gameFolder = "C:/Games";
    std::string imagePath = "C:/Images/game_icon.png";

    CreateGameFolder(gameFolder);
    CopyImageToFolder(imagePath, gameFolder);

    std::cout << "Game folder and image created successfully!" << std::endl;

    return 0;
}
