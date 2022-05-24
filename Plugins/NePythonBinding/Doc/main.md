# 这是什么

**NewEdenPythonBinding**（简称NEPY）是一个全新的UE4-Python插件，接入后可使用Python脚本编写游戏玩法逻辑。它可以替换官方的Plugins\Experimental\PythonScriptPlugin（处于实验阶段，仅为编辑器服务）。

NEPY具有以下特点：
1. 提供1比1的UE4类型、接口、属性导出，与C++类型、接口、属性保持一致。告别缺少接口的糟心事，告别接口用法不一致导致的学习困难。
2. 全自动代码生成，方便引擎版本升级后重新导出。
3. 完全使用Python C-API生成代码，没有借助boost-python或pybind11等第三方库。
4. 同时会生成引擎接口的Python Stub文件（.pyi），为编辑器（VSCode、PyCharm）提供代码提示。
5. 对于没有静态导出的类型、接口、属性，退化成类似UEP的运行时反射方式获取。
6. 内嵌Python2.7.16，不需依赖本地Python环境。

# 如何接入

## 使用虚幻官方UE4.27.2引擎
1. 使用EpicGamesLauncher->库->引擎版本->选择4.27.2进行安装。


## 接入步骤

在以下步骤中，将假设引擎位于```D:\UE4.27```，项目工程位于```D:\MyGame```，项目名为```MyGame.uproject```。

1. 将Plugins目录copy到D:\MyGame

	这样便为你的项目MyGame新增了二进制版NePy插件```D:\MyGame\Plugins\NePythonBinding```。

2. 将Scripts目录copy到D:\MyGame

	这样便为你的项目增加了只具有init，tick等简单功能的python脚本。

3. 配置GameInstance调用Scripts里的逻辑

	打开```MyGame.uproject```，将默认启动NePy插件（检查方法：按<kbd>Alt+`</kbd> 会弹出Python命令行窗口, 在命令行窗口中可以执行Python脚本）。
	Settings-> Project Settings -> Maps & Modes ->Game Instance Class 设置为 NePyGameInstance.

4. 接入成功

	点击Play, 将出现GameInstance start 的log(可通过Output Log窗口或者Python命令行窗口查看)。

	恭喜，接入成功！


# 基础用法

1. 根模块

	NEPY的根模块名为```ue```
	```python
	import ue
	```

2. 根模块接口访问
	[根模块手写Python接口](ManualPyInterface/NePy_ue.inl)
	NEPY的根模块名为```ue```
	```python
	ue.RequestExit()
	```

3. UClass访问接口
	已导出的类可以直接用ue.ClassName.Class()访问到，否则要用FindClass接口
	```python
	ue.Actor.Class()
	ue.FindClass("Actor")  # 等效与ue.Actor.Class()

	ue.FindClass("KismetAnimationLibrary")  # 没导出的Class不能用ue.KismetAnimationLibrary.Class()
	```

4. 普通成员函数访问
	具有反射功能的函数可以参考源码或者[UE4 CPP API文档](https://docs.unrealengine.com/4.27/en-US/API/)
	NePy也实现了一些手写接口方便大家使用，如[Actor手写Python接口](ManualPyInterface/NePyObject_Actor.inl)
	其他类的手写接口可以参考Client\Plugins\NePythonBinding\Doc\ManualPyInterface下的文件
	```python
	actor = ue.GetSelectedActors()[0]
	owner = actor.GetOwner()  # 反射接口，已被导出
	components = actor.GetComponents()  # 非反射接口，可以参考ManualPyInterface/NePyObject_Actor.inl
	```

5. 普通属性访问

	```python
	ue.GetSelectedActors()[0].RootComponent
	```
6. 静态函数访问

	```python
	# GameplayStatics
	ue.GameplayStatics.LoadStreamLevel()

	# KismetSystemLibrary
	ue.KismetSystemLibrary.GetEngineVersion()

	# 调用未导出类的静态方法
	KismetAnimationLibrary = ue.FindClass("KismetAnimationLibrary")
	KismetAnimationLibrary.K2_MakePerlinNoiseAndRemap(0, 0, 1)
	```

7. 在C++实现反射接口给Python使用
	若NePy的接口不能满足你的需求，可以再C++扩展出API供Python使用
	请参考Samples/PythonExtender样例
	```c++
	//c++中定义
	UCLASS()
	class UPythonExtenderExample : public UObject
	{

		GENERATED_UCLASS_BODY()
	public:
		UFUNCTION(BlueprintCallable)
		static FVector GetExampleLocation();

	};
	```
	```python
	# python中使用
	ue.FindClass("PythonExtenderExample").GetExampleLocation()
	```

8. UE4对象生命周期管理

	如果想要让Python持有UE4对象并阻止其被蓝图虚拟机GC：

	```python
	ue_object.OwnByPython()
	ue_object.DisownByPython()
	```

# 其他文档

- [创建Python游戏工程](init_python_project.html)

- [Subclassing文档](subclassing.html)

- [ui参考文档](ui.html)

- [API存根文档](ue.pyi) （可使用VSCode之类的编辑器打开）

- [UE4 CPP API文档](https://docs.unrealengine.com/4.27/en-US/API/)


# 使用样例
见Samples目录下的几个样例工程
