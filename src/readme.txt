配置参数说明
cityList  #城市列表 可选cq,tj,jx有多个城市的时候使用，隔开,未列出来的城市由于网站是没有历史数据所以，不做分析
topCount #显示结果数目1~256
cqIgnoreCount  #忽略比较老的N个记录 0~, 修改后需要重置数据库 ,cq是指重庆，若要改江西那么就是jxIgnoreCount
               #!!!!!注意：cq的历史数据都是规范的，但是jx有48个不规范，tj有1个，所以jxIgnoreCount系统会强制大于48,
               #!!!!!tjIgnoreCount系统会强制大于1,
cqUpdateTime #自动更新时间间隔 60s~300S，cq是指重庆,若要改江西那么就是jxIgnoreCount

