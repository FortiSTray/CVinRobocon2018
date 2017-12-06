### QR Code Locate

**程序流程**

1. 获取旋转矩形
2. 旋转矩形通过**尺寸**与**比例**预排除
3. find position pattern
   * 各方位1:1:3:1:1
   * 各区间0 / 255
4. 根据position pattern的位置定位QR Code
   * 正则化
5. 裁切

**待优化**

> ROI的裁取方式，不采用对角线裁取

角点检测

透视变换

