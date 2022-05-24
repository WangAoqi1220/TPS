# 简介

此章节将描述如何搭建Python脚本框架，以初始化游戏开发环境。

# 脚本目录

假如项目目录为```D:\MyGame```，则以下Scripts目录能被NEPY识别为脚本目录：

```
D:\
|--- MyGame
    |--- Scripts
    |--- Content
```

# 引擎初始化/销毁回调

NEPY在引擎启动后，会加载脚本根目录下的```ue_site.py```作为初始化脚本。

```ue_site.py```提供了若干回调函数，可作为脚本层的入口：

```python

def on_init():
    '''此函数会在NEPY插件初始化（StartupModule）时调用，可用于进行一些初始化工作。'''

def on_shutdown():
    '''此函数会在NEPY插件销毁（ShutdownModule）时调用，可用于进行一些清理工作。'''

def on_post_engine_init():
    '''此函数是可选函数。如果定义了此函数，则会在引擎初始化完毕（OnPostEngineInit）时调用。'''

def on_tick(delta_secodes):
    '''此函数是可选函数。如果定义了此函数，则会在每帧Tick的时候调用。'''

def on_debug_input(cmd_str):
    '''此函数是可选函数。如果定义了此函数，则当用户在PythonConsole中输入指令时，会首先回调到该函数。
    此函数可用于制作复杂的GM指令逻辑。返回值的含义是用户是否已自行处理了命令，若返回False，则PythonConsole会继续eval用户命令。'''
```

注意！只要NEPY成功加载，ue_site.py就会被执行。这不仅包括打包好的游戏，还包括**编辑器**，以及**Commandlet**。在编写ue_site.py时，需要注意区分这两种环境：

```python
# ue_site.py

def on_init():
    if ue.GIsEditor:
        # 当前是编辑器环境
    elif ue.IsRunningCommandlet():
        # 当前是命令行模式
    else:
        # 当前是纯游戏环境
```

# 游戏开始/结束回调

每次开始游戏时，UE4会创建全局唯一实例```UGameInstance```。我们可以使用以下步骤在```UGameInstance```上挂接Python脚本，来监听游戏的开始和结束：


1. 打开**项目设置**->**地图和模式**，将**游戏实例**设置为```NePyGameInstance```。

2. 在脚本根目录下新建Python脚本```ue_site.py```：

	```python
	# PyGameInstance.py

	class PyGameInstance(object):
	    def init(self):
	        '''如有定义，则在UGameInstance::Init()时调用。'''

	    def on_start(self):
	        '''如有定义，则在UGameInstance::OnStart()时调用，时机比init稍晚。'''

	    def shutdown(self):
	        '''如有定义，则在UGameInstance::Shutdown()时调用。'''

	    def on_tick(self, delta_secodes):
	        '''如有定义，则每帧调用。'''
	```

3. 运行游戏，PyGameInstance中的回调函数便会在对应时机被调用。