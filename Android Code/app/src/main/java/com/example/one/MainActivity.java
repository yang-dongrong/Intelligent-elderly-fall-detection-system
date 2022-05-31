package com.example.one;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class MainActivity extends AppCompatActivity {

    private String wd,sd;
    private int Devices=855363450;
    private String APi_Key="TPG=D5zq=GkaN4xPwW=Y0Q2dZio=";
    private TextView tem,status;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.setTitle("老人防摔倒");
        tem=findViewById(R.id.tem);
        status=findViewById(R.id.status);
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true){
                    wd=Get("temperature");
                    sd=Get("Zitai");
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            tem.setText(wd);
                            status.setText("1".equals(sd)?"摔倒":"正常");
                        }
                    });
                }
            }
        }).start();
    }
    private String Get(String name) {
        OkHttpClient okHttpClient = new OkHttpClient();
        Request request = new Request.Builder().url("https://api.heclouds.com/devices/" +
                "" + Devices + "/datapoints?datastream_id="+name).header("api-key"
                , APi_Key).build();
        try {
            Response response = okHttpClient.newCall(request).execute();
            JSONObject json1 = new JSONObject(response.body().string());
            JSONObject jsonObject1 = json1.getJSONObject("data");
            JSONArray jsonArray1 = jsonObject1.getJSONArray("datastreams");
            JSONObject json2 = new JSONObject(jsonArray1.get(0).toString());
            JSONArray jsonArray2 = json2.getJSONArray("datapoints");
            JSONObject json3 = new JSONObject(jsonArray2.get(0).toString());
            return  json3.getString("value");
        } catch (JSONException e) {
            e.printStackTrace();
        } catch (IOException ioException) {
            ioException.printStackTrace();
        }
        return "";
    }
}