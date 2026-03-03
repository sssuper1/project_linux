#!/bin/bash

# 定义源文件和目标设备
SOURCE_FILE="/home/root/BOOT.BIN"
TARGET_DEVICE="/dev/mtd0"
SOURCE_FILE2="/home/root/image.ub"
TARGET_DEVICE2="/dev/mtd2"

# 检查源文件是否存在
if [ ! -f "$SOURCE_FILE" ]; then
    echo "错误：源文件 $SOURCE_FILE 不存在!"
    exit 1
fi



# 执行flashcp命令，并显示详细输出
whoami > /www/test
echo $PATH >>/www/test
echo "正在将 $SOURCE_FILE 写入 $TARGET_DEVICE ..."
/usr/sbin/flashcp -v "$SOURCE_FILE" "$TARGET_DEVICE" >>/www/test
# 检查flashcp命令的退出状态
if [ $? -ne 0 ]; then
    echo "错误：写入过程中发生错误!"  >> /www/test

else
    echo "成功：$SOURCE_FILE 已成功写入 $TARGET_DEVICE" >> /www/test
fi

# 检查源文件是否存在
if [ ! -f "$SOURCE_FILE2" ]; then
    echo "错误：源文件 $SOURCE_FILE2 不存在!" >> /www/test
    
fi



# 执行flashcp命令，并显示详细输出
echo "正在将 $SOURCE_FILE2 写入 $TARGET_DEVICE2 ..."
/usr/sbin/flashcp -v "$SOURCE_FILE2" "$TARGET_DEVICE2" >>/www/test

# 检查flashcp命令的退出状态
if [ $? -ne 0 ]; then
    echo "错误：写入过程中发生错误!" >>/www/test
    exit 1
else
    echo "成功：$SOURCE_FILE2 已成功写入 $TARGET_DEVICE2" >>/www/test
fi

reboot


