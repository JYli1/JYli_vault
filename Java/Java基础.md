# 把基础语法过了一遍，开始面向对象了

# 静态问题

加static为静态方法。

在其它类调用方法时，如果有static则可以直接 类名.方法名 调用方法

如果没有static，则需要先new一个对象，再调用

静态方法和类一起加载，非静态方法在类加载之后加载

# 类 属性 方法 构造器（构造方法）

```java
public class 类名{

  String 属性名

  public viod 方法名(){
  
  }
  
  public 构造方法名{
    
  }
}
```

# 构造方法(构造器)

- 构造器名要和类名相同
- 没有返回值，且不要写void
- new类时会自动创建一个空的无参构造器
- 当创建有参构造器后，还要使用无参构造器就要自己手写一个
- 作用：

- 初始化属性
- new的本质就是调用构造器

# 封装

设置私有属性，设置公共方法，get/set方法来管理数据

# 继承

extends关键字

```java
public class studens extends Persons{

}
```

子类继承父类的公共属性和方法

所有类都默认继承object类

### super关键字

# 多态

父类可以new子类

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1757501784037-2c677936-de51-4609-9d2a-e70e69b8a1a3.png)

  

方法是多态的，属性不是

多态的条件是：有继承关系且有方法重写

## final static private 方法不能重写，所以不存在多态

  

  

# static 关键字详解

### 属性

有static为静态变量，静态变量可以通过类调用，非静态变量只能通过对象名调用。所以非静态变量调用前必须new一个对象

也就是说静态变量可以被更多类调用

### 方法

静态方法与类一起加载，非静态方法在类加载之后加载。

所以

1. 非静态方法可以直接 类名.方法名 调用，而非静态方法必须先new一个对象，在通过 对象名.方法名 调用。
2. 静态方法中不可以直接调用非静态方法，而非静态可直接以调用静态

  

### 代码块

匿名代码块 静态代码块

1. 静态代码块和类一起加载，最先执行
2. 匿名代码块第二执行
3. 构造方法第三执行

静态的东西只会在开始执行一次

# 抽象类 abstract

  

abstract关键字修饰

```java
public abstract class 类名{
    public abstract void 方法名(){

    }
}
```

- 约束作用，便于统一规范。
- 抽象方法必须抽象类，抽象类不一定统一方法
- 抽象类不能new，只能在子类中重写再new子类

  

# 接口 implenment

interface关键字 implenment关键字

```java
public interface 接口名{
    void 方法名 ();
}


public class 类名 implement 接口1 ,接口2{
    @override
    public void 方法名 (){

    }
}

```

接口不能new

  

# 内部类

```java
public class Outer {

    private int ID = 123456;
    
    public void out(){
        System.out.println("外部类");
    }

    public class Inner{
        public void in(){
            System.out.println("内部类");
        }  

        public void getID(){
            System.out.println(ID);//获取外部类的私有属性ID
        }
        
    }
    
}

//main方法调用内部方法时要先new外部类
Outer outer = new Outer();//实例化外部类
Outer.Inner inner = Outer.new Inner();//通过外部类的new方法实例化内部类
inner.in();//调用内部方法1



```

  

内部类可以获得外部类的私有属性和方法

  

# 异常机制 Exception

## Error

## Exception

  

## 关键字

1. try（一定要有）

监控区域

2. catch（一定要有）

捕获异常，出现异常时执行catch中的内容

3. finally（不一定要有）

善后工作，无论是否出现异常都会执行finally中的内容

4. throw

抛出异常

5. throws

```java
try{
    
}catch(Error e){  //"Error"为异常类型
     System.out.println("出现Error");//捕获异常后的操作
}catch(Throwable e){
     System.out.println("出现Throwable");
}finally{
     System.out.println("123");//不管怎么样都要做的
}


public void test(int a ,int b){
    if(b==0){
        throw new 异常类型;//主动怕抛出异常
    }
}
```

可以捕获多个异常

## 自定义异常

要继承Exception类

```java
public class MyException extends Exception{
    
}
```

# 多线程 java.Thread

  

进程process

线程thread

## 通过继承Thread类实现多线程

```java
public class test extends Thread{
    @override
    public void run(){
            
    }//重写run方法
    public static void main(String[] args){
        test test1 = new test();//实例化test类
        test1.star();//启动test1线程

        
    }
}


//1.自定义线程类继承Thread
//2.重写run方法
//3.创建线程对象，通过start方法启动线程
```

run()方法按顺序执行

start()方法，开启多线程

### 练习：网图下载

```java
package com.li.Demo01;

//import org.apache.commons.io.FileUtils;

import java.net.URL;
//写线程类
public class MyThread extends Thread {

    private String url;//图片地址
    private String fileName;//图片名称
    public MyThread(String url, String fileName){
        this.url = url;
        this.fileName = fileName;
    }
    public void run(){
        Dowdloader dowdloader = new Dowdloader();
        dowdloader.dowsloader(url, fileName);
        System.out.println("下载了文件："+fileName);
    }

    public static void main(String[] args) {
        //创建线程对象
        MyThread t1 = new MyThread("https://www.baidu.com/img/bd_logo1.png", "1.png");
        MyThread t2 = new MyThread("https://www.baidu.com/img/bd_logo1.png", "2.png");
        MyThread t3 = new MyThread("https://www.baidu.com/img/bd_logo1.png", "3.png");
        //开启线程
        t1.start();
        t2.start();
        t3.start();
    }
}

//下载器
class Dowdloader{
    public void downloadder(Srting url, String fileName){
        FileUtils.copyURLToFile(new URL( url), new File(fileName));
        //捕捉异常。。。。。。
    }
}
```

## 通过实现Runnable接口实现多线程（推荐）

```java
public class test implenment Runnable{
    @override
    public void run(){
            
    }//重写run方法
    public static void main(String[] args){
        //创建Runnable接口的实现对象
        test test1 = new test(); 
        //创建线程对象，通过线程对象来开启我们线程，代理
        // Thread thread1 = new Thread(test1);
        // thread.star();
        new Thread(test1).star();
        
    }
}
```

## 并发问题

```java
//抢火车票
public class test2 implenment Runnable{
    
    private int ticktnumbers = 10;//票数
    
    @override//重写run方法
    public void run(){
        while(){
            //退出循环条件
            if (ticitnumbers <= 0){
                break;
            }
            //捕捉异常
            try{
                Thread.sleep(200);//模拟抢票的延时 200ms
            }catch(){
                
            }
            
            System.out.println(Thread.currentThread().getName()+"-->抢到了第"+ ticktnumbers-- +"张票");
        }
    }
    
    
    public static void main(String[] args){
        //实例化接口对象
        test2 Test2 = new test2();
        //用同一个对象启动不同线程
        new thread(Test2,"老李").star();
        new thread(Test2,"小红").star();
        new thread(Test2,"黄牛").star();
    }
    
}
```

  

多线程操作统一资源时出现问题：线程不安全，同时抢到同一张票

## 龟兔赛跑

```java
public class Race implenment Runnable{
    
    private winner;//胜利者

    //重写run方法
    @Override
    public void run(){
        for(i = 1;i <= 100;i++){

            if(Thread.currentThread().getName().equals("兔子") && i > 50 && i % 10 == 0){
                //捕获异常
                try{
                    Thread.sleep(10);//模拟兔子睡觉
                }catch(){
                    
                }
            }

            //判断比赛结束，退出循环
            if(gameOver(i)){
                break;    
            }
            
            System.out.println(Thread.currentThread().getName() + "-->跑了" + i + "步");
        }
    }

    //判断是否比赛结束
    public boolean gameOver(int step){
        if(step >= 100){
            winner = Thread.currentThread().getName();
            System.out.println("winner is " + winner);
        }
        if(winner != null){
            return true;
        }
        return false;
    }

    //main方法
    public static void main(String[] args){
        Race race = new Race;
        
        new Thread(race,"乌龟").star();
        new Thread(race,"兔子").star();
    }
}
```


## 通过Callable接口


## 静态代理

```java
//婚庆公司

public class staticproxy{
    public static void main(String[] args){
        //让WeddingCompany代理You执行HappyMarry
        new WeddingCompany(new You).HappyMarry;
    }
}
//Marry接口，让You和WeddingCompany都要重写HappyMarry方法
interface Marry{
    void HappyMarry();
}

//You实现接口
class You implenment Marry{
    @Override
    public void HappyMarry(){
        System.out.println("...结婚了");
    }
}

//WeddingCompany实现接口
class WeddingCompany implenment Marry{
    private Marry target;//代理的目标

    //有参构造器
    public WeddingCompany(Marry target){
        this.target = target;
    }
    
    @Override
    public void HappyMarry(){
        before();
        this.target.HappyMarry();
        after();
    }
    
    private void before(){
        System.out.println("布置现场");
    }
    
    private void after(){
        System.out.println("收尾款");
    }
}
```

## Lambda表达式

1. 函数式接口：只有一个抽象方法的接口

```
(参数)->System.out.println("123");
```




## 线程停止

  

# 反射

每个类加载时会创建一个class类型的对象，反射就是通过这个class对象获取类的属性，结构

## 获取class类

1. 每个类都有class属性，用来获取class对象

所以用 `Class c1 = 类名.class` 可以获得class对象

2. 已经实例化的对象，可以`Class c2 = 对象名.getClass()` 获得类的class对象
3. 或用Class类的静态方法 Class.forName("包名.类名")
4. 通过获得的类 类.getSuperclass() 获得父类的class

## 类的初始化

### 会初始化

1. 虚拟机启动时自动初始化main
2. new一个对象时初始化类
3. 通过反射获取class时
4. 调用静态东西时
5. 调用子类时，父类没被初始化，会先初始化父类

### 不会初始化

1. 调用常量
2. 通过子类调用父类的静态

## 类加载器

（class）.getClassLoader

## 获取类的运行时结构

getName()——————获得包名+类名

getSimpleName()——————获得类名

getFields()——————获得public属性

getDeclaredFields()——————获得所有属性

getMethods()——————获得本类和父类的所有public方法

getDecleraMethods()——————获得本类的所有方法、

## 动态创建对象

调用Class对象的newInstance()方法创建对象

1. 类必须有一个无参构造器
2. 类的构造器访问权限足够

```
//获得一个Class对象
Class c1 = Class.forName("com.li.dasdad.User");
//构造一个对象
User u1 = (User)c1.newInstance();//本质就是调用了类的无参构造器，和new差不多

//通过构造器创建对象
//获得构造器
Constructor con1 = c1.getDeclaredConstructor(String.class,int.class,int.class);
//调用有参构造器
User u2 = (User)con1.newInstance(参数1，参数2，参数3);


//通过反射调用方法
User u3 = (User)c1.newInstance();//创建一个对象
c1.getDecalredMethod("SetName",String.class);//参数1为方法名，参数2为方法参数的类型
setName.invoke(u3,"lisongtao");//参数1为对象名，参数2为方法的参数


//通过反射调用属性
User u4 = (User)c1.newInstance();
Field name = c1.getDeclaredField("name");//获取字段，之后通过set方法操作

name.setAccessible(ture);//开启权限
name.set(user,"1234");

```

setAccessible(ture)方法，取消权限控制，设置后可以操作private属性或方法

invoke(1，2)方法：相当于激活方法，参数1为对象名，参数2为方法的参数

## 性能对比

new > 关闭检测反射 > 普通反射

## 通过反射操作泛型信息

  

  

# 代理模式

## 静态代理

```
//婚庆公司

public class staticproxy{
    public static void main(String[] args){
        //让WeddingCompany代理You执行HappyMarry
        new WeddingCompany(new You).HappyMarry;
    }
}
//Marry接口，让You和WeddingCompany都要重写HappyMarry方法
interface Marry{
    void HappyMarry();
}

//You实现接口
class You implenment Marry{
    @Override
    public void HappyMarry(){
        System.out.println("...结婚了");
    }
}

//WeddingCompany实现接口
class WeddingCompany implenment Marry{
    private Marry target;//代理的目标

    //有参构造器
    public WeddingCompany(Marry target){
        this.target = target;
    }
    
    @Override
    public void HappyMarry(){
        before();
        this.target.HappyMarry();
        after();
    }
    
    private void before(){
        System.out.println("布置现场");
    }
    
    private void after(){
        System.out.println("收尾款");
    }
}
```

## 动态代理

### 了解两个类

1. Proxy
2. InvocationHandler

### 代码

![](https://cdn.nlark.com/yuque/0/2025/png/51404470/1757986145302-ea622436-4fe4-4170-a104-44222b0dac96.png)