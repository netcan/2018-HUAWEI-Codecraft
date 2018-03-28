# author: netcan

from datetime import datetime
from codecraft_login import login, s
from lxml.html import fromstring
import time

def change_moto(username, password, moto="每日100次开奖机会用完了"):
    global s
    url = 'http://codecraft.devcloud.huaweicloud.com/User/UpdateTeamName'
    if not s:
        s = login(username, password)

    rvt = fromstring(s.get('http://codecraft.devcloud.huaweicloud.com/home/mycenter').text)
    try:
        rvt = rvt.cssselect('.myteam input[name=__RequestVerificationToken]')[0].value
    except IndexError:
        s = login(username, password)

    res = s.post(url, data=[
        ('teamName', '围墙 编队'),
        ('teamMoto', moto),
        ('__RequestVerificationToken', rvt)
    ], headers={
        'Referer': 'http://codecraft.devcloud.huaweicloud.com/home/mycenter',
        'DNT': '1',
    }).json()
    print(res)
    if res['Code'] != 0:
        s = login(username, password)
    return res


def auto_change_moto(username, password):
    dt = datetime(2018, 4, 16) - datetime.now()
    moto = '距离初赛截止还有{:.2f}秒'.format(dt.total_seconds())
    print(change_moto(username, password, moto))
    time.sleep(50)


if __name__ == '__main__':
    while True:
        auto_change_moto(None, None)
