import requests, time
from lxml.html import fromstring


def submit():
    import requests

    cookies = {
        '__RequestVerificationToken': 'qOz9WbRVwxXnQ-CbjppaNwnAW-DKSLTVT4ACMu8x21RBcfk7HL8WuZPVjYArNTuvjck5jLRQCsrA7W3Rhmk8cl33S241',
        'ASP.NET_SessionId': '02hnpbqxlwhyelky5oieqozr',
        '_ga': 'GA1.2.1883416146.1520406997',
        'vk': '0709218f-c2e5-4247-aa29-9fd7cbf198cd',
        '_dmpa_ref': '%5B%22%22%2C%22%22%2C1520407033%2C%22http%3A%2F%2Fwww.huaweicloud.com%2F%3Fticket%3DST-111080-dJ1nCdXhrsLZGwdB4AbJHOJn-sso%26locale%3Dzh-cn%22%5D',
        '_dmpa_ses': 'a154b4351ba3f3bbc97c28427bc78040397ab28b',
        'ad_sc': 'Baidu',
        'LastAdSource': 'Baidu',
        'ad_mdm': 'cpc',
        'ad_cmp': '%25E7%25A0%2594%25E7%25A9%25B6-%25E4%25BA%25A7%25E5%2593%2581-%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8',
        'ad_ctt': '%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8-%25E4%25BA%25A7%25E5%2593%2581',
        'ad_tm': '%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8',
        '_dmpa_id': '1214701a4914d204c9471856828631500689209192.1520407064.0.1520408147..',
        'SessionID': '65b24dff-42ed-42c9-9cd8-c4e8e59437b4',
        'LAT': '20180317123905',
        '.HAPApp': '2F9797C9F19F3C672BFAA010184060B3E7462EB2D26AF1A05DE9D572B2E5CFE0B43B5FA07A2135B893EED3A67CF4205F2B9B882DE7194E5AC4C9EC19ABA3A98EE79E054E655E9C349BF2426C12414115997764CD4D156D11E4486C146767B5F159AE5E30',
    }

    headers = {
        'Pragma': 'no-cache',
        'Origin': 'http://codecraft.devcloud.huaweicloud.com',
        'Accept-Encoding': 'gzip, deflate',
        'Accept-Language': 'zh-CN,zh;q=0.9,la;q=0.8,de;q=0.7',
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36',
        'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8',
        'Accept': 'application/json, text/javascript, */*; q=0.01',
        'Cache-Control': 'no-cache',
        'X-Requested-With': 'XMLHttpRequest',
        'Connection': 'keep-alive',
        'Referer': 'http://codecraft.devcloud.huaweicloud.com/home/mycenter',
        'DNT': '1',
    }

    data = [
        ('repositoryId', '195412'),
        ('__RequestVerificationToken',
         'xJ3if_w6hF2u6HW--hCiu-rtmTeMynMGjp6ASHiHURNW5bcyK72DsbhBFTE_zkq6h2qwnn-hw05d2IxopJ4YyCq011zznMeQLlK0jdhVvQT5kJ_00'),
    ]

    response = requests.post('http://codecraft.devcloud.huaweicloud.com/OtherSystem/DevCloudProjectSubmit',
                             headers=headers, cookies=cookies, data=data)


def get_score():
    import requests

    cookies = {
        '__RequestVerificationToken': 'qOz9WbRVwxXnQ-CbjppaNwnAW-DKSLTVT4ACMu8x21RBcfk7HL8WuZPVjYArNTuvjck5jLRQCsrA7W3Rhmk8cl33S241',
        'ASP.NET_SessionId': '02hnpbqxlwhyelky5oieqozr',
        '_ga': 'GA1.2.1883416146.1520406997',
        'vk': '0709218f-c2e5-4247-aa29-9fd7cbf198cd',
        '_dmpa_ref': '%5B%22%22%2C%22%22%2C1520407033%2C%22http%3A%2F%2Fwww.huaweicloud.com%2F%3Fticket%3DST-111080-dJ1nCdXhrsLZGwdB4AbJHOJn-sso%26locale%3Dzh-cn%22%5D',
        '_dmpa_ses': 'a154b4351ba3f3bbc97c28427bc78040397ab28b',
        'ad_sc': 'Baidu',
        'LastAdSource': 'Baidu',
        'ad_mdm': 'cpc',
        'ad_cmp': '%25E7%25A0%2594%25E7%25A9%25B6-%25E4%25BA%25A7%25E5%2593%2581-%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8',
        'ad_ctt': '%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8-%25E4%25BA%25A7%25E5%2593%2581',
        'ad_tm': '%25E4%25BA%2591%25E6%259C%258D%25E5%258A%25A1%25E5%2599%25A8',
        '_dmpa_id': '1214701a4914d204c9471856828631500689209192.1520407064.0.1520408147..',
        'SessionID': '65b24dff-42ed-42c9-9cd8-c4e8e59437b4',
        'LAT': '20180317123905',
        '.HAPApp': '2F9797C9F19F3C672BFAA010184060B3E7462EB2D26AF1A05DE9D572B2E5CFE0B43B5FA07A2135B893EED3A67CF4205F2B9B882DE7194E5AC4C9EC19ABA3A98EE79E054E655E9C349BF2426C12414115997764CD4D156D11E4486C146767B5F159AE5E30',
    }

    headers = {
        'Pragma': 'no-cache',
        'DNT': '1',
        'Accept-Encoding': 'gzip, deflate',
        'Accept-Language': 'zh-CN,zh;q=0.9,la;q=0.8,de;q=0.7',
        'Upgrade-Insecure-Requests': '1',
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36',
        'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8',
        'Cache-Control': 'no-cache',
        'Referer': 'http://codecraft.devcloud.huaweicloud.com/home/mycenter',
        'Connection': 'keep-alive',
    }

    response = requests.get('http://codecraft.devcloud.huaweicloud.com/home/mycenter', headers=headers, cookies=cookies)

    html = fromstring(response.text)
    return html.cssselect('.my-race-td3')[1].text_content()

    # NB. Original query string below. It seems impossible to parse and
    # reproduce query strings 100% accurately so the one below is given
    # in case the reproduced version is not "correct".
    # response = requests.get('http://codecraft.devcloud.huaweicloud.com/home/mycenter?m=2', headers=headers, cookies=cookies)


def auto_submit(score_threshold=86.9):
    ge_score = False
    score = 0
    while not ge_score:
        try:
            score = float(get_score())
            print(score)
            if score < score_threshold:
                submit()
            else:
                ge_score = True
        except ValueError:
            time.sleep(1)
            pass


if __name__ == '__main__':
    auto_submit()
