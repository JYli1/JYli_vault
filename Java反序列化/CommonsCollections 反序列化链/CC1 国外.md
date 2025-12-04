前面还是一样的通过`ChainedTransformer的transform`方法递归调用`InvokerTransformer的transform`方法从而实现命令执行。

那么我们现在的目标就是找到一个可以调用`ChainedTransformer的transform`方法的地方，这里我们找的是`LazyMap中的get`方法，而CC1中选择的是`TransformedMap中的checkSetvalue`
![](assets/CC1%20国外/file-20251204161845259.png)
我们只需要给`factory`参数赋值为`ChainedTransformer`就好了