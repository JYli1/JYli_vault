# 【电子木鱼】
给了源码，是Rust语言写的一个功德计算的程序，功德大于十亿得到flag
`/upgrade`路由通过传入的`body.name`变量对`GONGDE`进行各种操作，`PAYLOADS`数组变量包含五个不同的Payload结构体，`name`分别对应不同的操作
主要源码：
```rust
const PAYLOADS: &[Payload] = &[
    Payload {
        name: "Cost",
        cost: 10,
    },
    Payload {
        name: "Loan",
        cost: -1_000,
    },
    Payload {
        name: "CCCCCost",
        cost: 500,
    },
    Payload {
        name: "Donate",
        cost: 1,
    },
    Payload {
        name: "Sleep",
        cost: 0,
    },
];
 
#[post("/upgrade")]
async fn upgrade(body: web::Form<Info>) -> Json<APIResult> {
    if GONGDE.get() < 0 {
        return web::Json(APIResult {
            success: false,
            message: "功德都搞成负数了，佛祖对你很失望",
        });
    }
 
    if body.quantity <= 0 {
        return web::Json(APIResult {
            success: false,
            message: "佛祖面前都敢作弊，真不怕遭报应啊",
        });
    }
 
    if let Some(payload) = PAYLOADS.iter().find(|u| u.name == body.name) {
        let mut cost = payload.cost;
 
        if payload.name == "Donate" || payload.name == "Cost" {
            cost *= body.quantity;
        }
 
        if GONGDE.get() < cost as i32 {
            return web::Json(APIResult {
                success: false,
                message: "功德不足",
            });
        }
 
        if cost != 0 {
            GONGDE.set(GONGDE.get() - cost as i32);
        }
 
...
 
        }
    }
 
    web::Json(APIResult {
        success: false,
        message: "禁止开摆",
    })
}
```
存在漏洞的主要是这里：
```rust
if GONGDE.get() < cost as i32 {
            return web::Json(APIResult {
                success: false,
                message: "功德不足",
            });
        }
```
如果`cost`变量能够为负数，那么就能够加`GONGDE`，但问题是后端对传入的`body.quantity`进行了校验，而Rust作为一种安全性较高的语言，又很难绕过校验。

注意到后端对`cost`的数值类型限定为32位int，那么就有可能存在整型溢出漏洞。如果直接传入`2147483648`后端会报错。但由于cost进行了乘法操作`cost *= body.quantity;`，当`body.name=Cost`时，`cost`变量默认为`10`，因此我们传入`body.quantity=214748365`，乘法操作后cost就会变为`2147483650`，int32下会溢出为负数。
`（i32 能表示的最大值是2 147 483 647）`

通过这段代码扣钱：
```rust
if cost != 0 {

            GONGDE.set(GONGDE.get() - cost as i32);

        }
```
只要cost为一个很大的负数，我们就可以给GONGDE加很多了。所以才需要对整数溢出，`rust`中对超过最大值的数会从负的最大值开始`'环绕'` .也就是`2147483648`会等于`-2147483648`。因为int32：`最大：2147483647 最小：-2147483648`。
payload:
`post传：name=Cost&quantity=214748365`.
# 【象棋王子】
翻一下js文件，发现fuckjs代码：
![](assets/VNCTF%20%202023%20web%20复现/file-20251203193025605.png)
到控制台执行
![500](assets/VNCTF%20%202023%20web%20复现/file-20251203193025617.png)
