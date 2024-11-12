from flask import Flask, jsonify, Response
from flask_cors import CORS
import mysql.connector
from collections import OrderedDict
import json

app = Flask(__name__)

CORS(app, resources={r"/*": {"origins": "*"}})

mydb = None

def init_db_connection():
    global mydb
    if mydb is None or not mydb.is_connected():
        mydb = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            database="tb_cuaca"
        )
    return mydb

@app.route("/")
def read_root():
    return jsonify({"Hello": "World"})

@app.route("/Data")
def get_data():
    mydb = init_db_connection()
    cursor = mydb.cursor()
    try:
        # query buat humidity, temperature, lux, sama timestamp
        cursor.execute("SELECT id, suhu, humid, lux, ts FROM tb_cuaca")
        result = cursor.fetchall()
        
        # nge kestrak nilai temperature dan humidity untuk perhitungan
        temperature = [row[1] for row in result]
        humidity = [row[2] for row in result]

        # hitung si suhumax, suhumax, and suhurata
        suhumax = max(temperature) if temperature else None
        suhumin = min(temperature) if temperature else None
        suhurata = sum(temperature) / len(temperature) if temperature else None
        
        # siapkan nilai_suhu_max_humid_max data
        nilai_suhu_max_humid_max = [
            {
                "id": row[0],
                "suhu": row[1],
                "humid": row[2],
                "Lux": row[3],
                "Timestamp": row[4].strftime("%Y-%m-%d %H:%M:%S")
            }
            for row in result if row[1] == suhumax and row[2] == max(humidity)
        ]

        # siapkan data month_year_max dengan nge ekstrak month and year dari timestamps
        month_year_max = list({row[4].strftime("%m-%Y") for row in result})
        month_year_max = [{"month_year": my} for my in sorted(month_year_max, reverse=True)]
        
        # format si json nya agar urutan nya sesuai pake orderedict, 
        response = OrderedDict({
            "suhumax": suhumax,
            "suhumin": suhumin,
            "average": suhurata,
            "nilai_suhu_max_humid_max": nilai_suhu_max_humid_max,
            "month_year_max": month_year_max
        })
        
        # gunakan juga json.dumps agar urutannya terkunci
        return Response(json.dumps(response), mimetype='application/json')
    except mysql.connector.Error as e:
        return jsonify({"error": str(e)})
    finally:
        cursor.close()

if __name__ == "__main__":
    app.run(debug=True)
