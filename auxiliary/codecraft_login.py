# author: netcan

import requests
import json
from urllib.parse import unquote, urlparse, parse_qs


def login(username, password):
    url = 'https://auth.huaweicloud.com/authui/validateUser.action?service=https%3A%2F%2Fconsole.huaweicloud.com%2Fssoproxy%2Fauthticket%3Fservice%3Dhttp%253a%252f%252fcodecraft.devcloud.huaweicloud.com%252fauth%252fssocallback'
    s = requests.Session()
    s.headers.update({
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36',
    })

    print(s.post(url, data=[
        ('userpasswordcredentials.domain', ''),
        ('userpasswordcredentials.domainType', 'name'),
        ('userpasswordcredentials.username', username),
        ('userpasswordcredentials.countryCode', '0086'),
        ('userpasswordcredentials.verifycode', ''),
        ('userpasswordcredentials.password', password),
        ('userpasswordcredentials.companyLogin', 'false'),
        ('userpasswordcredentials.userInfoType', 'name'),
        ('__checkbox_warnCheck', 'true'),
        ('isAjax', 'true'),
        ('Submit', 'Login')
    ]).json())

    return s


