from codecraft_login import login, s
from auto_change_moto import change_moto
from lxml.html import fromstring
from datetime import datetime


def get_submit_count(username, password):
    global s
    if not s:
        s = login(username, password)
    html = fromstring(s.get("http://codecraft.devcloud.huaweicloud.com/home/mycenter").text)
    today = datetime.now().date()
    cnt = 0
    for dt in html.cssselect('.my-race-td1'):
        day = datetime.strptime(dt.text_content(), '%Y-%m-%d %H:%M:%S').date()
        if day == today:
            cnt += 1

    str = '今日提交次数：{}/100'.format(cnt)
    change_moto(username, password, str)
    print(str)
    return cnt


if __name__ == '__main__':
    pass
