# MCSManager 模板编辑器

这是一个用于编辑 MCSManager 服务端模板 JSON 文件的命令行工具。它允许你查看、添加、编辑和删除 Minecraft 服务端模板，无需图形界面，单个文件即可运行。

## 功能特点

- 查看模板中的语言列表
- 查看、添加、编辑和删除 Minecraft 服务端包
- 查看包的详细信息（仅显示描述信息）
- 保存更改到 JSON 文件
- 按语言保存包到单独文件
- 合并多个 JSON 文件
- 保留最新构建版本（自动识别并保留同一基础版本的最大构建号）
- 切换文件功能（支持保存当前更改并加载新的 JSON 文件）

## 快速启动

下载 mcsm_template_editor.exe ，在 json 文件夹内放入 .json文件，运行
```
mcsm_template_editor.exe json文件名
```
如
```
mcsm_template_editor.exe server.json
```
即可打开 server.json 文件进行编辑。


## 注意事项

- 读取或生成的文件都在json文件夹下，无需手动加上 json/ 前缀。
- 请在备份文件后使用本工具，以防意外情况发生。
- 本工具仅支持编辑 MCSManager 服务端模板 JSON 文件，不支持编辑其他类型的 JSON 文件。

## 贡献

如果您发现了任何问题或有改进建议，请随时提交 issue 。

## 许可证

MIT