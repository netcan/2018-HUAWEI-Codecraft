# author: netcan
from codecraft_login import login, s
from auto_change_moto import change_moto
from lxml.html import fromstring
from datetime import datetime
import math, time


def get_submit_count(username, password):
    global s
    if not s: s = login(username, password)
    html = fromstring(s.get("http://codecraft.devcloud.huaweicloud.com/home/mycenter").text)
    today = datetime.now().date()
    cnt = 0
    for dt in html.cssselect('.my-race-td1'):
        day = datetime.strptime(dt.text_content(), '%Y-%m-%d %H:%M:%S').date()
        if day == today:
            cnt += 1

    score = get_score(username, password) / 100
    str = '今日提交次数：{}/100，{:.5}²='.format(cnt, math.sqrt(score) * 100)
    change_moto(username, password, str)
    print(str)
    return cnt


def get_score(username, password):
    global s
    if not s: s = login(username, password)
    html = fromstring(s.get("http://codecraft.devcloud.huaweicloud.com/home/mycenter").text)
    return float(html.cssselect('.pcSroceBox .my-race-td3')[1].text_content())



if __name__ == '__main__':
    while True:
        get_submit_count('netcan', 'noah9168!')
        time.sleep(120)
