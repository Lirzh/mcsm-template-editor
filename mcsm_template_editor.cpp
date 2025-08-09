#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <regex>      // ����������ʽƥ��
#include <windows.h>  // ���ڴ���Ŀ¼
#include <filesystem> // ���ڼ��Ŀ¼�Ƿ����

namespace fs = std::filesystem;

// ����nlohmann/json��
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

// �������ݽṹ�Ը��õر�ʾJSON����
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

// ��JSON���������ݽṹ
TemplateData parseTemplateData(const json& j) {
    TemplateData data;
    data.remark = j.value("remark", "");

    // ��������
    if (j.contains("languages")) {
        for (const auto& lang : j["languages"]) {
            Language l;
            l.label = lang.value("label", "");
            l.value = lang.value("value", "");
            data.languages.push_back(l);
        }
    }

    // ������
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

            // ����setupInfo
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

// �����ݽṹת����JSON
json templateDataToJson(const TemplateData& data) {
    json j;
    j["remark"] = data.remark;

    // ת������
    json languagesJson = json::array();
    for (const auto& lang : data.languages) {
        json langJson;
        langJson["label"] = lang.label;
        langJson["value"] = lang.value;
        languagesJson.push_back(langJson);
    }
    j["languages"] = languagesJson;

    // ת����
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

        // ת��setupInfo
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

// ���ļ���ȡJSON
json readJsonFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        // ������jsonĿ¼�в����ļ�
        string jsonDir = "json";
        string jsonFilePath = jsonDir + "/" + filePath;
        file.open(jsonFilePath);
        if (!file.is_open()) {
            cerr << "�޷����ļ�: " << filePath << " �� " << jsonFilePath << endl;
            return json();
        }
    }

    json j;
    try {
        file >> j;
    } catch (const json::exception& e) {
        cerr << "JSON��������: " << e.what() << endl;
        return json();
    }

    file.close();
    return j;
}

// ����JSON���ļ�
bool saveJsonToFile(const json& j, const string& fileName) {
    // ȷ��jsonĿ¼����
    const string jsonDir = "json";
    if (!fs::exists(jsonDir)) {
        if (!fs::create_directory(jsonDir)) {
            cerr << "�޷�����jsonĿ¼" << endl;
            return false;
        }
    }

    // ���������ļ�·��
    string filePath;
    if (fileName.find(jsonDir + "/") == 0) {
        // ����ļ����Ѿ���json/��ͷ����ֱ��ʹ��
        filePath = fileName;
    } else {
        // �������json/ǰ׺
        filePath = jsonDir + "/" + fileName;
    }

    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "�޷����ļ�����д��: " << filePath << endl;
        return false;
    }

    try {
        file << j.dump(2); // ��ʽ�����������2���ո�
    } catch (const json::exception& e) {
        cerr << "JSON���л�����: " << e.what() << endl;
        return false;
    }

    file.close();
    cout << "�ѳɹ����浽�ļ�: " << filePath << endl;
    return true;
}

// �����Խ������浽�����ļ�
void savePackagesByLanguage(const TemplateData& data) {
    // �����Է���
    map<string, vector<Package>> packagesByLang;
    for (const auto& pkg : data.packages) {
        packagesByLang[pkg.language].push_back(pkg);
    }

    // Ϊÿ�����Դ���������JSON�ļ�
    for (const auto& [lang, packages] : packagesByLang) {
        json j;
        j["remark"] = "Packages for language: " + lang;
        j["language"] = lang;

        // ���Ҷ�Ӧ�����Ա�ǩ
        for (const auto& language : data.languages) {
            if (language.value == lang) {
                j["language_label"] = language.label;
                break;
            }
        }

        // ת����
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

            // ת��setupInfo
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

        // ���浽�ļ�
        string fileName = lang + "_packages.json";
        saveJsonToFile(j, fileName);
    }

    cout << "�Ѱ����Խ������浽�����ļ�!\n";
}

// ��ȡ�汾�ź͹�����
bool extractVersionAndBuild(const string& title, string& baseVersion, int& buildNumber) {
    // ƥ���ʽ�� "Minecraft 1.10.2 build918" �ı���
    regex pattern(R"((.+?)\s+build(\d+))");
    smatch match;
    
    if (regex_search(title, match, pattern)) {
        baseVersion = match[1];
        buildNumber = stoi(match[2]);
        return true;
    }
    return false;
}

// ����ͬһ�汾�����¹����汾
void keepLatestBuildVersions(TemplateData& data) {
    // �������汾�����Է���
    map<pair<string, string>, vector<Package>> packagesByBaseVersionAndLang;
    
    for (const auto& pkg : data.packages) {
        string baseVersion;
        int buildNumber;
        
        if (extractVersionAndBuild(pkg.title, baseVersion, buildNumber)) {
            packagesByBaseVersionAndLang[{baseVersion, pkg.language}].push_back(pkg);
        } else {
            // ���Ǵ�build�ŵİ汾��ֱ�ӱ���
            packagesByBaseVersionAndLang[{pkg.title, pkg.language}].push_back(pkg);
        }
    }
    
    // ���ԭ�а��б�
    data.packages.clear();
    
    // ����ÿ�飬ֻ����build�����İ�
    for (auto& [key, packages] : packagesByBaseVersionAndLang) {
        if (packages.size() == 1) {
            // ֻ��һ������ֱ�����
            data.packages.push_back(packages[0]);
        } else {
            // ��������ҵ�build������
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
            cout << "�ѱ������°汾: " << latestPackage.title << endl;
        }
    }
    
    cout << "��������¹����汾�ı�����\n";
}

// �ϲ�����JSON�ļ�
void mergeJsonFiles(TemplateData& data) {
    cout << "����Ҫ�ϲ���JSON�ļ�·��: ";
    string mergeFilePath;
    getline(cin, mergeFilePath);

    json mergeJ = readJsonFromFile(mergeFilePath);
    if (mergeJ.is_null() || mergeJ.empty()) {
        cout << "�޷���ȡ�����Ҫ�ϲ���JSON�ļ�.\n";
        return;
    }

    // ����Ҫ�ϲ�������
    TemplateData mergeData = parseTemplateData(mergeJ);

    // �ϲ�����
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

    // �ϲ���
    for (const auto& pkg : mergeData.packages) {
        bool exists = false;
        for (auto& existingPkg : data.packages) {
            // ����ͨ��������������ж��Ƿ�Ϊ��ͬ�İ�
            if (existingPkg.title == pkg.title && existingPkg.language == pkg.language) {
                exists = true;
                // �������а�
                existingPkg = pkg;
                break;
            }
        }
        if (!exists) {
            data.packages.push_back(pkg);
        }
    }

    cout << "�ѳɹ��ϲ�JSON�ļ�!\n";
}

// ��ʾ�����б�
void displayLanguages(const vector<Language>& languages) {
    cout << "���������б�:\n";
    for (size_t i = 0; i < languages.size(); ++i) {
        cout << "  " << i + 1 << ". " << languages[i].label << " (" << languages[i].value << ")\n";
    }
}

// ��ʾ���б�ֻ��ʾ������
void displayPackages(const vector<Package>& packages) {
    cout << "���ð��б�:\n";
    for (size_t i = 0; i < packages.size(); ++i) {
        cout << "  " << i + 1 << ". " << packages[i].description << "\n";
    }
}

// ��ʾ������
void displayPackageDetails(const Package& pkg) {
    cout << "������:\n";
    cout << "  ����: " << pkg.title << "\n";
    cout << "  ����: " << pkg.language << "\n";
    cout << "  ��Ϸ����: " << pkg.gameType << "\n";
    cout << "  ����: " << pkg.description << "\n";
    cout << "  ����: " << pkg.category << "\n";
    cout << "  ����ʱ: " << pkg.runtime << "\n";
    cout << "  Ӳ��Ҫ��: " << pkg.hardware << "\n";
    cout << "  ��С: " << pkg.size << "\n";
    cout << "  ��ע: " << pkg.remark << "\n";
    cout << "  ��������: " << pkg.targetLink << "\n";
    cout << "  ����: " << pkg.author << "\n";
    cout << "  ������Ϣ:\n";
    cout << "    ����: " << pkg.setupInfo.type << "\n";
    cout << "    ��������: " << pkg.setupInfo.startCommand << "\n";
    cout << "    ֹͣ����: " << pkg.setupInfo.stopCommand << "\n";
    cout << "    ��������: " << pkg.setupInfo.updateCommand << "\n";
    cout << "    �������: " << pkg.setupInfo.ie << "\n";
    cout << "    �������: " << pkg.setupInfo.oe << "\n";
}

// ����°�
void addNewPackage(vector<Package>& packages) {
    Package newPkg;

    cout << "�������Ϣ:\n";
    cout << "����: ";
    getline(cin, newPkg.language);
    cout << "��Ϸ����: ";
    getline(cin, newPkg.gameType);
    cout << "����: ";
    getline(cin, newPkg.description);
    cout << "����: ";
    getline(cin, newPkg.title);
    cout << "����: ";
    getline(cin, newPkg.category);
    cout << "����ʱ: ";
    getline(cin, newPkg.runtime);
    cout << "Ӳ��Ҫ��: ";
    getline(cin, newPkg.hardware);
    cout << "��С: ";
    getline(cin, newPkg.size);
    cout << "��ע: ";
    getline(cin, newPkg.remark);
    cout << "��������: ";
    getline(cin, newPkg.targetLink);
    cout << "����: ";
    getline(cin, newPkg.author);

    cout << "������Ϣ:\n";
    cout << "����: ";
    getline(cin, newPkg.setupInfo.type);
    cout << "��������: ";
    getline(cin, newPkg.setupInfo.startCommand);
    cout << "ֹͣ����: ";
    getline(cin, newPkg.setupInfo.stopCommand);
    cout << "��������: ";
    getline(cin, newPkg.setupInfo.updateCommand);
    cout << "�������: ";
    getline(cin, newPkg.setupInfo.ie);
    cout << "�������: ";
    getline(cin, newPkg.setupInfo.oe);

    packages.push_back(newPkg);
    cout << "���ѳɹ����!\n";
}

// �༭���а�
void editPackage(vector<Package>& packages) {
    displayPackages(packages);
    cout << "����Ҫ�༭�İ���� (0ȡ��): ";
    int index;
    cin >> index;
    cin.ignore(); // ���Ի��з�

    if (index <= 0 || index > static_cast<int>(packages.size())) {
        cout << "ȡ���༭.\n";
        return;
    }

    Package& pkg = packages[index - 1];
    displayPackageDetails(pkg);

    cout << "�����µİ���Ϣ (ֱ�ӻس�����ԭֵ):\n";

    string input;
    cout << "���� [" << pkg.language << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.language = input;

    cout << "��Ϸ���� [" << pkg.gameType << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.gameType = input;

    cout << "���� [" << pkg.description << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.description = input;

    cout << "���� [" << pkg.title << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.title = input;

    cout << "���� [" << pkg.category << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.category = input;

    cout << "����ʱ [" << pkg.runtime << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.runtime = input;

    cout << "Ӳ��Ҫ�� [" << pkg.hardware << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.hardware = input;

    cout << "��С [" << pkg.size << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.size = input;

    cout << "��ע [" << pkg.remark << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.remark = input;

    cout << "�������� [" << pkg.targetLink << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.targetLink = input;

    cout << "���� [" << pkg.author << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.author = input;

    cout << "������Ϣ:\n";
    cout << "���� [" << pkg.setupInfo.type << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.type = input;

    cout << "�������� [" << pkg.setupInfo.startCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.startCommand = input;

    cout << "ֹͣ���� [" << pkg.setupInfo.stopCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.stopCommand = input;

    cout << "�������� [" << pkg.setupInfo.updateCommand << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.updateCommand = input;

    cout << "������� [" << pkg.setupInfo.ie << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.ie = input;

    cout << "������� [" << pkg.setupInfo.oe << "]: ";
    getline(cin, input);
    if (!input.empty()) pkg.setupInfo.oe = input;

    cout << "���ѳɹ�����!\n";
}

// ɾ����
void deletePackage(vector<Package>& packages) {
    displayPackages(packages);
    cout << "����Ҫɾ���İ���� (0ȡ��): ";
    int index;
    cin >> index;
    cin.ignore(); // ���Ի��з�

    if (index <= 0 || index > static_cast<int>(packages.size())) {
        cout << "ȡ��ɾ��.\n";
        return;
    }

    packages.erase(packages.begin() + index - 1);
    cout << "���ѳɹ�ɾ��!\n";
}

// ���˵�
void showMainMenu() {
    cout << "\n===== MCSManager ģ��༭�� =====\n";
    cout << "1. �鿴�����б�\n";
    cout << "2. �鿴���б�\n";
    cout << "3. �鿴������\n";
    cout << "4. ����°�\n";
    cout << "5. �༭��\n";
    cout << "6. ɾ����\n";
    cout << "7. �����Ա�����������ļ�\n";
    cout << "8. �ϲ�JSON�ļ�\n";
    cout << "9. �������¹����汾\n";
    cout << "10. �������\n";
    cout << "11. �л��ļ�\n";
    cout << "12. �˳�\n";
    cout << "=================================\n";
    cout << "��ѡ�����: ";
}

int main(int argc, char* argv[]) {
    cout << "��ӭʹ�� MCSManager ģ��༭��!\n";

    // ȷ��jsonĿ¼����
    const string jsonDir = "json";
    if (!fs::exists(jsonDir)) {
        if (!fs::create_directory(jsonDir)) {
            cerr << "�޷�����jsonĿ¼" << endl;
            return 1;
        }
    }

    // �������в�����ȡ�ļ�·��������ʹ��Ĭ��·��
    string fileName = "templates.json";
    if (argc > 1) {
        fileName = argv[1];
    }

    // ���������ļ�·��
    string filePath = jsonDir + "/" + fileName;

    // ��ȡJSON�ļ�
    json j = readJsonFromFile(filePath);
    if (j.is_null() || j.empty()) {
        // ����ļ������ڻ�Ϊ�գ�����һ���µ�ģ��
        cout << "δ�ҵ���ЧJSON�ļ���������ģ��.\n";
        j = json({
            {"remark", "This file is only used for quick deployment."},
            {"languages", json::array({
                {"label", "English"}, {"value", "en_us"},
                {"label", "��������"}, {"value", "zh_cn"}
            })},
            {"packages", json::array()}
        });
    }

    // ���������ݽṹ
    TemplateData data = parseTemplateData(j);

    bool running = true;
    while (running) {
        showMainMenu();
        int choice;
        cin >> choice;
        cin.ignore(); // ���Ի��з�

        switch (choice) {
            case 1:
                displayLanguages(data.languages);
                break;
            case 2:
                displayPackages(data.packages);
                break;
            case 3: {
                displayPackages(data.packages);
                cout << "����Ҫ�鿴�İ���� (0ȡ��): ";
                int index;
                cin >> index;
                cin.ignore();
                if (index > 0 && index <= static_cast<int>(data.packages.size())) {
                    displayPackageDetails(data.packages[index - 1]);
                } else {
                    cout << "ȡ���鿴.\n";
                }
                break;
            }
            case 4:
                addNewPackage(data.packages);
                break;
            case 5:
                if (data.packages.empty()) {
                    cout << "û�пɱ༭�İ�.\n";
                } else {
                    editPackage(data.packages);
                }
                break;
            case 6:
                if (data.packages.empty()) {
                    cout << "û�п�ɾ���İ�.\n";
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
                // ת����JSON������
                j = templateDataToJson(data);
                saveJsonToFile(j, filePath);
                break;
            case 11: {
                cout << "�Ƿ񱣴浱ǰ�ļ��ĸ���? (y/n): ";
                char saveChoice;
                cin >> saveChoice;
                if (saveChoice == 'y' || saveChoice == 'Y') {
                    j = templateDataToJson(data);
                    saveJsonToFile(j, filePath);
                }
                
                cout << "����Ҫ�л���JSON�ļ�·�� (ֱ�ӻس��������˵�): ";
                string newFilePath;
                getline(cin, newFilePath);
                getline(cin, newFilePath); // ��ȡʵ������
                
                if (!newFilePath.empty()) {
                    // ���Զ�ȡ���ļ�
                    json newJ = readJsonFromFile(newFilePath);
                    if (newJ.is_null() || newJ.empty()) {
                        cout << "�޷���ȡ��������ļ������ֵ�ǰ�ļ�.\n";
                    } else {
                        j = newJ;
                        data = parseTemplateData(j);
                        filePath = newFilePath;
                        cout << "�ѳɹ��л����ļ�: " << filePath << endl;
                    }
                }
                break;
            }
            case 12:
                cout << "�Ƿ񱣴����? (y/n): ";
                char exitSaveChoice;
                cin >> exitSaveChoice;
                if (exitSaveChoice == 'y' || exitSaveChoice == 'Y') {
                    j = templateDataToJson(data);
                    saveJsonToFile(j, filePath);
                }
                running = false;
                break;
            default:
                cout << "��Чѡ��������.\n";
                break;
        }
    }

    cout << "��лʹ�ã��ټ�!\n";
    return 0;
}