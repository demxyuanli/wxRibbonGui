#include "config/Coin3DConfig.h"
#include "config/ConfigManager.h"
#include "logger/Logger.h"
#include <gtest/gtest.h>
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

class Coin3DConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置临时配置文件路径
        wxString tempDir = wxStandardPaths::Get().GetTempDir();
        wxString testConfigPath = tempDir + wxFileName::GetPathSeparator() + "test_config.ini";
        
        // 初始化配置管理器
        ConfigManager::getInstance().initialize(testConfigPath.ToStdString());
    }

    void TearDown() override {
        // 清理临时配置文件
        wxString tempDir = wxStandardPaths::Get().GetTempDir();
        wxString testConfigPath = tempDir + wxFileName::GetPathSeparator() + "test_config.ini";
        wxRemoveFile(testConfigPath);
    }
};

// 测试场景图路径配置
TEST_F(Coin3DConfigTest, SceneGraphPath) {
    auto& config = Coin3DConfig::getInstance();
    
    // 测试默认值
    EXPECT_EQ(config.getSceneGraphPath(), "");
    
    // 测试设置和获取值
    std::string testPath = "/test/path/scene.iv";
    config.setSceneGraphPath(testPath);
    EXPECT_EQ(config.getSceneGraphPath(), testPath);
}

// 测试自动保存配置
TEST_F(Coin3DConfigTest, AutoSave) {
    auto& config = Coin3DConfig::getInstance();
    
    // 测试默认值
    EXPECT_TRUE(config.getAutoSaveEnabled());
    EXPECT_EQ(config.getAutoSaveInterval(), 5);
    
    // 测试设置和获取值
    config.setAutoSaveEnabled(false);
    config.setAutoSaveInterval(10);
    
    EXPECT_FALSE(config.getAutoSaveEnabled());
    EXPECT_EQ(config.getAutoSaveInterval(), 10);
}

// 测试默认材质配置
TEST_F(Coin3DConfigTest, DefaultMaterial) {
    auto& config = Coin3DConfig::getInstance();
    
    // 测试默认值
    EXPECT_EQ(config.getDefaultMaterial(), "Default");
    
    // 测试设置和获取值
    std::string testMaterial = "PhongMaterial";
    config.setDefaultMaterial(testMaterial);
    EXPECT_EQ(config.getDefaultMaterial(), testMaterial);
}

// 测试配置持久化
TEST_F(Coin3DConfigTest, ConfigPersistence) {
    auto& config = Coin3DConfig::getInstance();
    
    // 设置一些值
    std::string testPath = "/test/path/scene.iv";
    bool testAutoSave = false;
    int testInterval = 15;
    std::string testMaterial = "TestMaterial";
    
    config.setSceneGraphPath(testPath);
    config.setAutoSaveEnabled(testAutoSave);
    config.setAutoSaveInterval(testInterval);
    config.setDefaultMaterial(testMaterial);
    
    // 重新加载配置
    ConfigManager::getInstance().reload();
    
    // 验证值是否保持不变
    EXPECT_EQ(config.getSceneGraphPath(), testPath);
    EXPECT_EQ(config.getAutoSaveEnabled(), testAutoSave);
    EXPECT_EQ(config.getAutoSaveInterval(), testInterval);
    EXPECT_EQ(config.getDefaultMaterial(), testMaterial);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    wxApp::SetInstance(new wxApp());
    wxEntryStart(argc, argv);
    int result = RUN_ALL_TESTS();
    wxEntryCleanup();
    return result;
} 