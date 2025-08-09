#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <regex>      // 用于正则表达式匹配
#include <windows.h>  // 用于创建目录
#include <filesystem> // 用于检查目录是否存在

namespace fs = std::filesystem;

// 引入nlohmann/json库
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

// 定义数据结构以更好地表示JSON数据
struct Language {
    string label;
    string value;
};

struct SetupInfo {
    string type;
    string startCommand;
    string stopCommand;
    string updateCommand;
    string ie;
    string oe;
};

struct Package {
    string language;
    string gameType;
    string description;
    string title;
    string category;
    string runtime;
    string hardware;
    string size;
    string remark;
    string targetLink;
    string author;
    SetupInfo setupInfo;
};

struct TemplateData {
    string remark;
    vector<Language> languages;
    vector<Package> packages;
};

// 从JSON解析到数据结构
TemplateData parseTemplateData(const json& j) {
    TemplateData data;
    data.remark = j.value("remark", "");

    // 解析语言
    if (j.contains("languages")) {
        for (const auto& lang : j["languages"]) {
            Language l;
            l.label = lang.value("label", "");
            l.value = lang.value("value", "");
            data.languages.push_back(l);
        }
    }

    // 解析包
    if (j.contains("packages")) {
        for (const auto& pkg : j["packages"]) {
            Package p;
            p.language = pkg.value("language", "");
            p.gameType = pkg.value("gameType", "");
            p.description = pkg.value("description", "");
            p.title = pkg.value("title", "");
            p.category = pkg.value("category", "");
            p.runtime = pkg.value("runtime", "");
            p.hardware = pkg.value("hardware", "");
            p.size = pkg.value("size", "");
            p.remark = pkg.value("remark", "");
            p.targetLink = pkg.value("targetLink", "");
            p.author = pkg.value("author", "");

            // 解析setupInfo
            if (pkg.contains("setupInfo")) {
                const auto& setup = pkg["setupInfo"];
                p.setupInfo.type = setup.value("type", "");
                p.setupInfo.startCommand = setup.value("startCommand", "");
                p.setupInfo.stopCommand = setup.value("stopCommand", "");
                p.setupInfo.updateCommand = setup.value("updateCommand", "");
                p.setupInfo.ie = setup.value("ie", "");
                p.setupInfo.oe = setup.value("oe", "");
            }

            data.packages.push_back(p);
        }
    }

    return data;
}

// 从数据结构转换到JSON
json templateDataToJson(const TemplateData& data) {
    json j;
    j["remark"] = data.remark;

    // 转换语言
    json languagesJson = json::array();
    for (const auto& lang : data.languages) {
        json langJson;
        langJson["label"] = lang.label;
        langJson["value"] = lang.value;
        languagesJson.push_back(langJson);
    }
    j["languages"] = languagesJson;

    // 转换包
    json packagesJson = json::array();
    for (const auto& pkg : data.packages) {
        json pkgJson;
        pkgJson["language"] = pkg.language;
        pkgJson["gameType"] = pkg.gameType;
        pkgJson["description"] = pkg.description;
        pkgJson["title"] = pkg.title;
        pkgJson["category"] = pkg.category;
        pkgJson["runtime"] = pkg.runtime;
        pkgJson["hardware"] = pkg.hardware;
        pkgJson["size"] = pkg.size;
        pkgJson["remark"] = pkg.remark;
        pkgJson["targetLink"] = pkg.targetLink;
        pkgJson["author"] = pkg.author;

        // 转换setupInfo
        json setupJson;
        setupJson["type"] = pkg.setupInfo.type;
        setupJson["startCommand"] = pkg.setupInfo.startCommand;
        setupJson["stopCommand"] = pkg.setupInfo.stopCommand;
        setupJson["updateCommand"] = pkg.setupInfo.updateCommand;
        setupJson["ie"] = pkg.setupInfo.ie;
        setupJson["oe"] = pkg.setupInfo.oe;
        pkgJson["setupInfo"] = setupJson;

        packagesJson.push_back(pkgJson);
    }
    j["packages"] = packagesJson;

    return j;
}

// 从文件读取JSON
json readJsonFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        // 尝试在json目录中查找文件
        string jsonDir = "json";
        string jsonFilePath = jsonDir + "/" + filePath;
        file.open(jsonFilePath);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filePath << " 或 " << jsonFilePath << endl;
            return json();
        }
    }

    json j;
    try {
        file >> j;
    } catch (const json::exception& e) {
        cerr << "JSON解析错误: " << e.what() << endl;
        return json();
    }

    file.close();
    return j;
}

// 保存JSON到文件
bool saveJsonToFile(const json& j, const string& fileName) {
    // 确保json目录存在
    const string jsonDir = "json";
    if (!fs::exists(jsonDir)) {
        if (!fs::create_directory(jsonDir)) {
            cerr << "无法创建json目录" << endl;
            return false;
        }
    }

    // 构建完整文件路径
    string filePath;
    if (fileName.find(jsonDir + "/") == 0) {
        // 如果文件名已经以json/开头，则直接使用
        filePath = fileName;
    } else {
        // 否则添加json/前缀
        filePath = jsonDir + "/" + fileName;
    }

    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "无法打开文件进行写入: " << filePath << endl;
        return false;
    }

    try {
        file << j.dump(2); // 格式化输出，缩进2个空格
    } catch (const json::exception& e) {
        cerr << "JSON序列化错误: " << e.what() << endl;
        return false;
    }

    file.close();
    cout << "已成功保存到文件: " << filePath << endl;
    return true;
}

// 按语言将包保存到单独文件
void savePackagesByLanguage(const TemplateData& data) {
    // 按语言分组
    map<string, vector<Package>> packagesByLang;
    for (const auto& pkg : data.packages) {
        packagesByLang[pkg.language].push_back(pkg);
    }

    // 为每种语言创建并保存JSON文件
    for (const auto& [lang, packages] : packagesByLang) {
        json j;
        j["remark"] = "Packages for language: " + lang;
        j["language"] = lang;

        // 查找对应的语言标签
        for (const auto& language : data.languages) {
            if (language.value == lang) {
                j["language_label"] = language.label;
                break;
            }
        }

        // 转换包
        json packagesJson = json::array();
        for (const auto& pkg : packages) {
            json pkgJson;
            pkgJson["language"] = pkg.language;
            pkgJson["gameType"] = pkg.gameType;
            pkgJson["description"] = pkg.description;
            pkgJson["title"] = pkg.title;
            pkgJson["category"] = pkg.category;
            pkgJson["runtime"] = pkg.runtime;
            pkgJson["hardware"] = pkg.hardware;
            pkgJson["size"] = pkg.size;
            pkgJson["remark"] = pkg.remark;
            pkgJson["targetLink"] = pkg.targetLink;
            pkgJson["author"] = pkg.author;

            // 转换setupInfo
            json setupJson;
            setupJson["type"] = pkg.setupInfo.type;
            setupJson["startCommand"] = pkg.setupInfo.startCommand;
            setupJson["stopCommand"] = pkg.setupInfo.stopCommand;
            setupJson["updateCommand"] = pkg.setupInfo.updateCommand;
            setupJson["ie"] = pkg.setupInfo.ie;
            setupJson["oe"] = pkg.setupInfo.oe;
            pkgJson["setupInfo"] = setupJson;

            packagesJson.push_back(pkgJson);
        }
        j["packages"] = packagesJson;

        // 保存到文件
        string fileName = lang + "_packages.json";
        saveJsonToFile(j, fileName);
    }

    cout << "已按语言将包保存到单独文件!\n";
}

// 提取版本号和构建号
bool extractVersionAndBuild(const string& title, string& baseVersion, int& buildNumber) {
    // 匹配格式如 "Minecraft 1.10.2 build918" 的标题
    regex pattern(R"((.+?)\s+build(\d+))");
    smatch match;
    
    if (regex_search(title, match, pattern)) {
        baseVersion = match[1];
        buildNumber = stoi(match[2]);
        return true;
    }
    return false;
}

// 保留同一版本的最新构建版本
void keepLatestBuildVersions(TemplateData& data) {
    // 按基础版本和语言分组
    map<pair<string, string>, vector<Package>> packagesByBaseVersionAndLang;
    
    for (const auto& pkg : data.packages) {
        string baseVersion;
        int buildNumber;
        
        if (extractVersionAndBuild(pkg.title, baseVersion, buildNumber)) {
            packagesByBaseVersionAndLang[{baseVersion, pkg.language}].push_back(pkg);
        } else {
            // 不是带build号的版本，直接保留
            packagesByBaseVersionAndLang[{pkg.title, pkg.language}].push_back(pkg);
        }
    }
    
    // 清空原有包列表
    data.packages.clear();
    
    // 对于每组，只保留build号最大的包
    for (auto& [key, packages] : packagesByBaseVersionAndLang) {
        if (packages.size() == 1) {
            // 只有一个包，直接添加
            data.packages.push_back(packages[0]);
        } else {
            // 多个包，找到build号最大的
            Package latestPackage = packages[0];
            int maxBuildNumber = 0;
            string baseVersion;
            
            extractVersionAndBuild(latestPackage.title, baseVersion, maxBuildNumber);
            
            for (size_t i = 1; i < packages.size(); ++i) {
                int currentBuildNumber;
                string currentBaseVersion;
                
                if (extractVersionAndBuild(packages[i].title, currentBaseVersion, currentBuildNumber)) {
                    if (currentBuildNumber > maxBuildNumber) {
                        maxBuildNumber = currentBuildNumber;
                        latestPackage = packages[i];
                    }
                }
            }
            
            data.packages.push_back(latestPackage);
            cout << "已保留最新版本: " << latestPackage.title << endl;
        }
    }
    
    cout << "已完成最新构建版本的保留！\n";
}

// 合并两个JSON文件
void mergeJsonFiles(TemplateData& data) {
    cout << "输入要合并的JSON文件路径: ";
    string mergeFilePath;
    getline(cin, mergeFilePath);

    json mergeJ = readJsonFromFile(mergeFilePath);
    if (mergeJ.is_null() || mergeJ.empty()) {
        cout << "无法读取或解析要合并的JSON文件.\n";
        return;
    }

    // 解析要合并的数据
    TemplateData mergeData = parseTemplateData(mergeJ);

    // 合并语言
    for (const auto& lang : mergeData.languages) {
        bool exists = false;
        for (const auto& existingLang : data.languages) {
            if (existingLang.value == lang.value) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            data.languages.push_back(lang);
        }
    }

    // 合并包
    for (const auto& pkg : mergeData.packages) {
        bool exists = false;
        for (auto& existingPkg : data.packages) {
            // 假设通过标题和语言来判断是否为相同的包
            if (existingPkg.title == pkg.title && existingPkg.language == pkg.language) {
                exists = true;
                // 更新现有包
                existingPkg = pkg;
                break;
            }
        }
        if (!exists) {
            data.packages.push_back(pkg);
        }
    }

    cout << "已成功合并JSON文件!\n";
}

// 显示语言列表
void displayLanguages(const vector<Language>& languages) {
    cout << "可用语言列表:\n";
    for (size_t i = 0; i < languages.size(); ++i) {
        cout << "  " << i + 1 << ". " << languages[i].label << " (" << languages[i].value << ")\n";
    }
}

// 显示包列表（只显示描述）
void displayPackages(const vector<Package>& packages) {
    cout << "可用包列表:\n";
    for (size_t i = 0; i < packages.size(); ++i) {
        cout << "  " << i + 1 << ". " << packages[i].description << "\n";
    }
}

// 显示包详情
void displayPackageDetails(const Package& pkg) {
    cout << "包详情:\n";
    cout << "  标题: " << pkg.title << "\n";
    cout << "  语言: " << pkg.language << "\n";
    cout << "  游戏类型: " << pkg.gameType << "\n";
    cout << "  描述: " << pkg.description << "\n";
    cout << "  分类: " << pkg.category << "\n";
    cout << "  运行时: " << pkg.runtime << "\n";
    cout << "  硬件要求: " << pkg.hardware << "\n";
    cout << "  大小: " << pkg.size << "\n";
    cout << "  备注: " << pkg.remark << "\n";
    cout << "  下载链接: " << pkg.targetLink << "\n";
    cout << "  作者: " << pkg.author << "\n";
    cout << "  设置信息:\n";
    cout << "    类型: " << pkg.setupInfo.type << "\n";
    cout << "    启动命令: " << pkg.setupInfo.startCommand << "\n";
    cout << "    停止命令: " << pkg.setupInfo.stopCommand << "\n";
    cout << "    更新命令: " << pkg.setupInfo.updateCommand << "\n";
    cout << "    输入编码: " << pkg.setupInfo.ie << "\n";
    cout << "    输出编码: " << pkg.setupInfo.oe << "\n";
}

// 添加新包
void addNewPackage(vector<Package>& packages) {
    Package newPkg;

    cout << "输入包信息:\n";
    cout << "语言: ";
    getline(cin, newPkg.language);
    cout << "游戏类型: ";
    getline(cin, newPkg.gameType);
    cout << "描述: ";
    getline(cin, newPkg.description);
    cout << "标题: ";
    getline(cin, newPkg.title);
    cout << "分类: ";
    getline(cin, newPkg.category);
    cout << "运行时: ";
    getline(cin, newPkg.runtime);
    cout << "硬件要求: ";
    getline(cin, newPkg.hardware);
    cout << "大小: ";
    getline(cin, newPkg.size);
    cout << "备注: ";
    getline(cin, newPkg.remark);
    cout << "下载链接: ";
    getline(cin, newPkg.targetLink);
    cout << "作者: ";
    getline(cin, newPkg.author);

    cout << "设置信息:\n";
    cout << "类型: ";
    getline(cin, newPkg.setupInfo.type);
    cout << "启动命令: ";
    getline(cin, newPkg.setupInfo.startCommand);
    cout << "停止命令: ";
    getline(cin, newPkg.setupInfo.stopCommand);
    cout << "更新命令: ";
    getline(cin, newPkg.setupInfo.updateCommand);
    cout << "输入编码: ";
    getline(cin, newPkg.setupInfo.ie);
    cout << "输出编码: ";
    getline(cin, newPkg.setupInfo.oe);

    packages.push_back(newPkg);
    cout << "包已成功添加!\n";
}

// 编辑现有包
void editPackage(vector<Package>& packages) {
    displayPackages(packages);
    cout << "输入要编辑的包编号 (0取消): ";
    int index;
    cin >> index;
    cin.ignore(); // 忽略换行符

    if (index <= 0 || index > static_cast<int>(packages.size())) {
        cout << "取消编辑.\n";
        return;
    }

    Package& pkg = packages[index - 1];
    displayPackageDetails(pkg);

    cout << "输入新的包信息 (直接回车保留原值):\n";

    string input;
    cout << "语言 [" << pkg.language << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.language = input;

    cout << "游戏类型 [" << pkg.gameType << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.gameType = input;

    cout << "描述 [" << pkg.description << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.description = input;

    cout << "标题 [" << pkg.title << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.title = input;

    cout << "分类 [" << pkg.category << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.category = input;

    cout << "运行时 [" << pkg.runtime << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.runtime = input;

    cout << "硬件要求 [" << pkg.hardware << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.hardware = input;

    cout << "大小 [" << pkg.size << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.size = input;

    cout << "备注 [" << pkg.remark << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.remark = input;

    cout << "下载链接 [" << pkg.targetLink << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.targetLink = input;

    cout << "作者 [" << pkg.author << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.author = input;

    cout << "设置信息:\n";
    cout << "类型 [" << pkg.setupInfo.type << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.type = input;

    cout << "启动命令 [" << pkg.setupInfo.startCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.startCommand = input;

    cout << "停止命令 [" << pkg.setupInfo.stopCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.stopCommand = input;

    cout << "更新命令 [" << pkg.setupInfo.updateCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.updateCommand = input;

    cout << "输入编码 [" << pkg.setupInfo.ie << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.ie = input;

    cout << "输出编码 [" << pkg.setupInfo.oe << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.oe = input;

    cout << "包已成功更新!\n";
}

// 删除包
void deletePackage(vector<Package>& packages) {
    displayPackages(packages);
    cout << "输入要删除的包编号 (0取消): ";
    int index;
    cin >> index;
    cin.ignore(); // 忽略换行符

    if (index <= 0 || index > static_cast<int>(packages.size())) {
        cout << "取消删除.\n";
        return;
    }

    packages.erase(packages.begin() + index - 1);
    cout << "包已成功删除!\n";
}

// 主菜单
void showMainMenu() {
    cout << "\n===== MCSManager 模板编辑器 =====\n";
    cout << "1. 查看语言列表\n";
    cout << "2. 查看包列表\n";
    cout << "3. 查看包详情\n";
    cout << "4. 添加新包\n";
    cout << "5. 编辑包\n";
    cout << "6. 删除包\n";
    cout << "7. 按语言保存包到单独文件\n";
    cout << "8. 合并JSON文件\n";
    cout << "9. 保留最新构建版本\n";
    cout << "10. 保存更改\n";
    cout << "11. 切换文件\n";
    cout << "12. 退出\n";
    cout << "=================================\n";
    cout << "请选择操作: ";
}

int main(int argc, char* argv[]) {
    cout << "欢迎使用 MCSManager 模板编辑器!\n";

    // 确保json目录存在
    const string jsonDir = "json";
    if (!fs::exists(jsonDir)) {
        if (!fs::create_directory(jsonDir)) {
            cerr << "无法创建json目录" << endl;
            return 1;
        }
    }

    // 从命令行参数获取文件路径，或者使用默认路径
    string fileName = "templates.json";
    if (argc > 1) {
        fileName = argv[1];
    }

    // 构建完整文件路径
    string filePath = jsonDir + "/" + fileName;

    // 读取JSON文件
    json j = readJsonFromFile(filePath);
    if (j.is_null() || j.empty()) {
        // 如果文件不存在或为空，创建一个新的模板
        cout << "未找到有效JSON文件，创建新模板.\n";
        j = json({
            {"remark", "This file is only used for quick deployment."},
            {"languages", json::array({
                {"label", "English"}, {"value", "en_us"},
                {"label", "简体中文"}, {"value", "zh_cn"}
            })},
            {"packages", json::array()}
        });
    }

    // 解析到数据结构
    TemplateData data = parseTemplateData(j);

    bool running = true;
    while (running) {
        showMainMenu();
        int choice;
        cin >> choice;
        cin.ignore(); // 忽略换行符

        switch (choice) {
            case 1:
                displayLanguages(data.languages);
                break;
            case 2:
                displayPackages(data.packages);
                break;
            case 3: {
                displayPackages(data.packages);
                cout << "输入要查看的包编号 (0取消): ";
                int index;
                cin >> index;
                cin.ignore();
                if (index > 0 && index <= static_cast<int>(data.packages.size())) {
                    displayPackageDetails(data.packages[index - 1]);
                } else {
                    cout << "取消查看.\n";
                }
                break;
            }
            case 4:
                addNewPackage(data.packages);
                break;
            case 5:
                if (data.packages.empty()) {
                    cout << "没有可编辑的包.\n";
                } else {
                    editPackage(data.packages);
                }
                break;
            case 6:
                if (data.packages.empty()) {
                    cout << "没有可删除的包.\n";
                } else {
                    deletePackage(data.packages);
                }
                break;
            case 7:
                savePackagesByLanguage(data);
                break;
            case 8:
                mergeJsonFiles(data);
                break;
            case 9:
                keepLatestBuildVersions(data);
                break;
            case 10:
                // 转换回JSON并保存
                j = templateDataToJson(data);
                saveJsonToFile(j, filePath);
                break;
            case 11: {
                cout << "是否保存当前文件的更改? (y/n): ";
                char saveChoice;
                cin >> saveChoice;
                if (saveChoice == 'y' || saveChoice == 'Y') {
                    j = templateDataToJson(data);
                    saveJsonToFile(j, filePath);
                }
                
                cout << "输入要切换的JSON文件路径 (直接回车返回主菜单): ";
                string newFilePath;
                getline(cin, newFilePath);
                getline(cin, newFilePath); // 读取实际输入
                
                if (!newFilePath.empty()) {
                    // 尝试读取新文件
                    json newJ = readJsonFromFile(newFilePath);
                    if (newJ.is_null() || newJ.empty()) {
                        cout << "无法读取或解析新文件，保持当前文件.\n";
                    } else {
                        j = newJ;
                        data = parseTemplateData(j);
                        filePath = newFilePath;
                        cout << "已成功切换到文件: " << filePath << endl;
                    }
                }
                break;
            }
            case 12:
                cout << "是否保存更改? (y/n): ";
                char exitSaveChoice;
                cin >> exitSaveChoice;
                if (exitSaveChoice == 'y' || exitSaveChoice == 'Y') {
                    j = templateDataToJson(data);
                    saveJsonToFile(j, filePath);
                }
                running = false;
                break;
            default:
                cout << "无效选择，请重试.\n";
                break;
        }
    }

    cout << "感谢使用，再见!\n";
    return 0;
}