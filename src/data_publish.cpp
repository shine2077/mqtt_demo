// data_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to collect and publish periodic data to MQTT, as
// an MQTT publisher using the C++ asynchronous client interface.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Publishing messages
//  - Using a topic object to repeatedly publish to the same topic.
//  - Automatic reconnects
//  - Off-line buffering
//  - User file-based persistence with simple encoding.
//
// This just uses the steady clock to run a periodic loop. Each time
// through, it generates a random number [0-100] as simulated data and
// creates a text, CSV payload in the form:
//  	<sample #>,<time stamp>,<data>
//
// Note that it uses the steady clock to pace the periodic timing, but then
// reads the system_clock to generate the timestamp for local calendar time.
//
// The sample number is just a counting integer to help test the off-line
// buffering to easily confirm that all the messages got across.
//

/*******************************************************************************
 * Copyright (c) 2013-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/


#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>


#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "mqtt/async_client.h"
#include <cbor.h>

using namespace std;
using namespace std::chrono;

const std::string DFLT_ADDRESS { "tcp://47.95.150.143:1883" };
const std::string CLIENT_ID { "mqttclient" };

const string TOPIC { "data/rand" };
const int	 QOS = 1;

const auto PERIOD = seconds(3);

const int MAX_BUFFERED_MSGS = 100;	// 100 * 3sec => 5min off-line buffering

/////////////////////////////////////////////////////////////////////////////

#include "sqliteclient/sqliteclient.h"


// Example of sqlite3 persistence.
class encoded_sqlite3_persistence : virtual public mqtt::iclient_persistence
{
	// The name of the db
	// Used as the database name
	string dbname_;

	// sqlite client
	SQLiteClient* sqlite_client;

    // encoding
    void encode(string& s) const {
        for(auto &c : s){
            if(c == '\000'){
                c = '\040';
            }
        }
    }

    // decoding
    void decode(string& s) const {
        for(auto &c : s){
            if(c == '\040'){
                c = '\000';
            }
        }
    }


public:
	// Create the persistence object with the specified encoding key
	encoded_sqlite3_persistence()
			: sqlite_client(new SQLiteClient) {}

    virtual ~encoded_sqlite3_persistence(){
        delete sqlite_client;
    }

	// "Open" the persistence store.
	// Create a directory for persistence files, using the client ID and
	// serverURI to make a unique directory name. Note that neither can be
	// empty. In particular, the app can't use an empty `clientID` if it
	// wants to use persistence. (This isn't an absolute rule for your own
	// persistence, but you do need a way to keep data from different apps
	// separate).
	void open(const string& clientId, const string& serverURI) override {
		if (clientId.empty() || serverURI.empty())
			throw mqtt::persistence_exception();

		dbname_ = clientId+".db3";

        sqlite_client->open(dbname_);
        sqlite_client->createTable("test", false);
	}

	// Close the persistent database that was previously opened.
	void close() override {
		delete sqlite_client;
	}

	// Clear Table.
	void clear() override {
		sqlite_client->clearTable();
	}

	// Returns whether or not data is persisted using the specified key.
	// We just look for a file in the store directory with the same name as
	// the key.
	bool contains_key(const string& key) override {
		return sqlite_client->contains_key(key);
	}

	// Returns the keys in this persistent data store.
	mqtt::string_collection keys() const override {
		mqtt::string_collection ks;
		vector<string> keys_ = sqlite_client->keys();
		for(auto key_ : keys_){
            ks.push_back(key_);
        }
		return ks;
	}

	// Puts the specified data into the persistent store.
	// We just encode the data and write it to a file using the key as the
	// name of the file. The multiple buffers given here need to be written
	// in order - and a scatter/gather like writev() would be fine. But...
	// the data will be read back as a single buffer, so here we first
	// concat a string so that the encoding key lines up with the data the
	// same way it will on the read-back.
	void put(const string& key, const std::vector<mqtt::string_view>& bufs) override {
        string s;
        for (const auto& b : bufs){
            s.append(b.data(), b.size());
        }

        encode(s);

		sqlite_client->instert_date(key, s);
	}

	// Gets the specified data out of the persistent store.
	// We look for a file with the name of the key, read the contents,
	// decode, and return it.
	string get(const string& key) const override {
		string s = sqlite_client->getValue(key);

        decode(s);
        return s;
	}

	// Remove the data for the specified key.
	// Just remove the file with the same name as the key, if found.
	void remove(const string &key) override {
        sqlite_client->removeKey(key);
	}
};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	string address = (argc > 1) ? string(argv[1]) : DFLT_ADDRESS;

    encoded_sqlite3_persistence persist;

    auto clientOpts = mqtt::create_options_builder()
		.restore_messages(true)
		.max_buffered_messages(MAX_BUFFERED_MSGS)
		.mqtt_version(3)
		.finalize();

    mqtt::async_client cli(address, CLIENT_ID, clientOpts, &persist);


	auto connOpts = mqtt::connect_options_builder()
		.keep_alive_interval(MAX_BUFFERED_MSGS * PERIOD)
		.clean_session(true)
		.automatic_reconnect(true)
		.finalize();

	// Create a topic object. This is a conventience since we will
	// repeatedly publish messages with the same parameters.
	mqtt::topic top(cli, TOPIC, QOS, true);

	// Random number generator [0 - 100]
	random_device rnd;
    mt19937 gen(rnd());
    uniform_int_distribution<> dis(0, 100);

	try {
		// Connect to the MQTT broker
		cout << "Connecting to server '" << address << "'..." << flush;
		cli.connect(connOpts)->wait();
		cout << "OK\n" << endl;

		char tmbuf[32];
		unsigned nsample = 0;

		// The time at which to reads the next sample, starting now
		auto tm = steady_clock::now();

		while (true) {
			// Pace the samples to the desired rate
			this_thread::sleep_until(tm);

            // Get a timestamp and format as a string
            time_t t = system_clock::to_time_t(system_clock::now());

            /* Preallocate the map structure */
            cbor_item_t* root = cbor_new_definite_map(3);
            /* Add the content */
            bool success = cbor_map_add(
                    root, (struct cbor_pair){
                            .key = cbor_move(cbor_build_string("motor_speed")),
                            .value = cbor_move(cbor_build_float4(812.00))});
            success &= cbor_map_add(
                    root, (struct cbor_pair){
                            .key = cbor_move(cbor_build_string("fuel_level")),
                            .value = cbor_move(cbor_build_float4(73.12))});
            success &= cbor_map_add(
                    root, (struct cbor_pair){
                            .key = cbor_move(cbor_build_string("timestamp")),
                            .value = cbor_move(cbor_build_uint32(t))});
            if (!success) return 1;
            /* Output: `length` bytes of data in the `buffer` */
            unsigned char* buffer;
            size_t buffer_size;
            cbor_serialize_alloc(root, &buffer, &buffer_size);

            string payload(reinterpret_cast<char*>(buffer));

            cbor_decref(&root);

            /* Assuming `buffer` contains `info.st_size` bytes of input data */
//            struct cbor_load_result result;
//            cbor_item_t * item = cbor_load(buffer, buffer_size, &result);
//            /* Pretty-print the result */
//            cbor_describe(item, stdout);
//            fflush(stdout);
//            /* Deallocate the result */
//            cbor_decref(&item);

			// Publish to the topic
			top.publish(std::move(payload));

			tm += PERIOD;
		}

		// Disconnect
		cout << "\nDisconnecting..." << flush;
		cli.disconnect()->wait();
		cout << "OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}

