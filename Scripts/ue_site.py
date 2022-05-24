# -*- coding: utf-8 -*-
# from Network.network import NetworkSocket


def on_debug_input(cmd_str):
	'''此函数是可选函数。如果定义了此函数，则当用户在PythonConsole中输入指令时，会首先回调到该函数。
	此函数可用于制作复杂的GM指令逻辑。返回值的含义是用户是否已自行处理了命令，若返回False，则PythonConsole会继续eval用户命令。'''
	print('cmd_str', cmd_str)


def on_init():
	'''此函数会在NEPY插件初始化（StartupModule）时调用，可用于进行一些初始化工作。为最早回调的python接口'''
	print('on_nepy_init')


def on_post_engine_init():
	'''此函数是可选函数。如果定义了此函数，则会在引擎初始化完毕（OnPostEngineInit）时调用。回调时机晚于on_init'''
	print('on_post_engine_init')


# 下面的接口调用时机比on_post_engine_init要晚
# 将ProjectSettings里的GameInstanceClass设置为NePyGameInstance，下面的接口才会被回调。
class PyGameInstance(object):
	def init(self):
		'''如有定义，则在UGameInstance::Init()时调用。'''
		print('GameInstance init')

	def on_start(self):
		'''如有定义，则在UGameInstance::OnStart()时调用，时机比init稍晚。'''
		print('GameInstance start')

		# An example of a simple tcp networking test
		# self.socket = NetworkSocket()
		# self.socket.setup_socket()

	def on_tick(self, delta_seconds):
		'''如有定义，则每帧调用。'''
		print('GameInstance on_tick')
		# self.socket.tick(delta_seconds)

	def shutdown(self):
		'''如有定义，则在UGameInstance::Shutdown()时调用。'''
		print('GameInstance shutdown')
		# self.socket.destroy()
